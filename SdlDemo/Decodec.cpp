#include "stdafx.h"
#include "Decodec.h"


FfmpegDecodec* FfmpegDecodec::instance;//��ʼ��
//sdl
class MySDL;
static bool threadExit = true;
static bool startPushEvent = true;
static int delayTimemsec = 40;//25֡
FfmpegDecodec::FfmpegDecodec()
{
	//SDL_Thread *onsdlev_thread = SDL_CreateThread(&onSDLEventFfm, NULL, NULL);//�߳̽����¼�֪ͨ����֡����˽���Ͳ��Ῠס�����϶������
}
FfmpegDecodec::~FfmpegDecodec()
{
	//��Ҫ��������������������������Ӧ�������ʹ�����ͷŶ����Ǵ�����
}
FfmpegDecodec* FfmpegDecodec::getInstance()
{
	if (nullptr == instance)
		instance = new(std::nothrow)FfmpegDecodec();
	return instance;
};
int FfmpegDecodec::format2stream(AVFormatContext *pFormatCtx, char filePath[], int &videoIndex)
{
	av_log(NULL, AV_LOG_INFO, "format2stream,begin...");
	if (avformat_open_input(&pFormatCtx, filePath, NULL, NULL) != 0) {
		av_log(NULL, AV_LOG_ERROR, "Couldn't open input stream.");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		printf("Couldn't find stream information.\n");
		return -1;
	}
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoIndex = i;
			break;
		}
	if (videoIndex == -1) {
		printf("Didn't find a video stream.\n");
		return -1;
	}
	return 0;
}
int FfmpegDecodec::stream2frame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx)
{
	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		printf("Codec not found.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec.\n");
		return -1;
	}
	return 0;
}
int FfmpegDecodec::packet2H264(AVFormatContext *pFormatCtx, FILE *file, int videoIndex)
{
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	while (av_read_frame(pFormatCtx, packet) >= 0) {
		if (packet->stream_index == videoIndex) {
			/*
			* ���H264������ȡ����packet
			*/
			fwrite(packet->data, 1, packet->size, file);
		}
		av_free_packet(packet);
	}
	return 0;
}
int FfmpegDecodec::frame2YUV(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx
	, FILE *file, int videoIndex)
{
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	AVFrame *pFrame = av_frame_alloc();
	AVFrame *pFrameYUV = av_frame_alloc();
	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

	struct SwsContext* img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	int frame_cnt = 0, got_picture = -1;
	while (av_read_frame(pFormatCtx, packet) >= 0) {
		if (packet->stream_index == videoIndex) {

			int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0) {
				printf("Decode Error.\n");
				return -1;
			}
			if (got_picture) {
				//ȥ���Ҳ�ĺ�ɫ��ͼ�񣬺�Ӳ�����
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameYUV->data, pFrameYUV->linesize);
				printf("Decoded frame index: %d\n", frame_cnt);

				/*
				* ������YUV�Ĵ��룬ȡ����pFrameYUV��ʹ��fwrite()
				*/
				//pFrameYUV��data��Y\U\V ����������ɵ�����
				fwrite(pFrameYUV->data[0], 1, pCodecCtx->width*pCodecCtx->height, file);//Y����
				fwrite(pFrameYUV->data[1], 1, pCodecCtx->width*pCodecCtx->height / 4, file);//Y����
				fwrite(pFrameYUV->data[2], 1, pCodecCtx->width*pCodecCtx->height / 4, file);//Y����
				frame_cnt++;
			}
		}
	}
	av_free_packet(packet);
	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrameYUV);
	return 0;
}
int FfmpegDecodec::frame2YUVEX(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx
	, int videoIndex,int msec)
{
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	AVFrame *pFrame = av_frame_alloc();
	AVFrame *pFrameYUV = av_frame_alloc();
	uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

	struct SwsContext* img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	//�¼�������ȡH264֡
	int frame_cnt = 0, got_picture = -1;
	int screen_w = pCodecCtx->width;
	int screen_h = pCodecCtx->height;
	bool flag = true;
	SDL_Window* screen = nullptr;
	SDL_Thread *refresh_thread = SDL_CreateThread(&refDecVideo, NULL, NULL);//SDL�߳̽����¼�֪ͨ����֡����˽���Ͳ��Ῠס�����϶������
	mySDLInstance = MySDL::getInstance();
	do{
	    SDL_WaitEvent(&m_event);
	    switch (m_event.type)
	    {
	    case REFRESH_EVENT://����ÿһ֡
	    {
		    while (av_read_frame(pFormatCtx, packet) >= 0) {
			    if (packet->stream_index == videoIndex) {

				    int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
				    if (ret < 0) {
					    printf("Decode Error.\n");
					    return -1;
				    }
				    if (got_picture) {
					//ȥ���Ҳ�ĺ�ɫ��ͼ�񣬺�Ӳ�����
					sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
						pFrameYUV->data, pFrameYUV->linesize);
					printf("Decoded frame index: %d\n", frame_cnt);

					/*
					* ������YUV�Ĵ��룬ȡ����pFrameYUV��ʹ��fwrite()
					*/
					ffmSendYUV2SDL(pFrameYUV->data[0], pFrameYUV->linesize[0], 0, 0, pCodecCtx->width
						, pCodecCtx->height, screen_w, screen_h, msec);
				 	frame_cnt++;
				    }
				    break;
			    }
			    else //����Ƶ֡
			    {
			 	    continue;
			    }
		    }
		    break;
	    }
	    case SDL_WINDOWEVENT:
		//���ô������ڴ�С�ĸı�
		//SDL_GetWindowSize(screen, &screen_w, &screen_h);
		    break;
	    case SDL_QUIT:
		    threadExit = false;
		    break;
	    case BREAK_EVENT:
		    //8 �˳�SDL
		    SDL_Quit();
		    break;
	    case SDL_KEYUP:
	    {
		    switch (m_event.key.keysym.sym)
		    {
		    case SDLK_SPACE:
			    startPushEvent = !startPushEvent;
			    break;
		    case SDLK_ESCAPE:
			    SDL_Quit();
			    break;
		    case SDLK_RIGHT:
			    delayTimemsec = delayTimemsec - 5;
			    if (delayTimemsec < 5)
			    {
				    delayTimemsec = 5;
			    }
			    break;
		    case SDLK_LEFT:
			    delayTimemsec = delayTimemsec + 5;
			    break;
		    default:
			    break;
		    }
		    break;
	    }
	    default:
		    break;
	    }
	}while(flag);
	av_free_packet(packet);
	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrameYUV);
	return 0;
}
void FfmpegDecodec::ffmSendYUV2SDL(void *data, int pitch, int x, int y, int pixel_w, int pixel_h
	, int screen_w, int screen_h, int msec)
{
	int Ret = mySDLInstance->sendYUV2SDLDirect(data, pitch, 0, 0, pixel_w, pixel_h
		, screen_w, screen_h, msec);
}
int FfmpegDecodec::refDecVideo(void *) {//�ββ��ܶ�
	threadExit = true;
	while (threadExit) {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		if(startPushEvent)
		SDL_PushEvent(&event);
		SDL_Delay(delayTimemsec);//ÿ��40���뷢��һ��ˢ���¼�
	}
	threadExit = true;
	//Break
	SDL_Event event;
	event.type = BREAK_EVENT;//�����˳��ź�
	SDL_PushEvent(&event);
	return 0;
}