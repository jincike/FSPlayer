#include "stdafx.h"
#include "Decodec.h"


FfmpegDecodec* FfmpegDecodec::instance;//初始化
//sdl
class MySDL;
static bool threadExit = true;
static bool startPushEvent = true;
static int delayTimemsec = 40;//25帧
FfmpegDecodec::FfmpegDecodec()
{
	//SDL_Thread *onsdlev_thread = SDL_CreateThread(&onSDLEventFfm, NULL, NULL);//线程进行事件通知更新帧，如此界面就不会卡住，可拖动鼠标了
}
FfmpegDecodec::~FfmpegDecodec()
{
	//不要在析构函数中析构单例。单例应该由类的使用者释放而不是创造者
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
			* 输出H264码流，取自于packet
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
				//去除右侧的黑色的图像，和硬件相关
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameYUV->data, pFrameYUV->linesize);
				printf("Decoded frame index: %d\n", frame_cnt);

				/*
				* 添加输出YUV的代码，取自于pFrameYUV，使用fwrite()
				*/
				//pFrameYUV的data由Y\U\V 三个分量组成的数组
				fwrite(pFrameYUV->data[0], 1, pCodecCtx->width*pCodecCtx->height, file);//Y数据
				fwrite(pFrameYUV->data[1], 1, pCodecCtx->width*pCodecCtx->height / 4, file);//Y数据
				fwrite(pFrameYUV->data[2], 1, pCodecCtx->width*pCodecCtx->height / 4, file);//Y数据
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
	//事件驱动，取H264帧
	int frame_cnt = 0, got_picture = -1;
	int screen_w = pCodecCtx->width;
	int screen_h = pCodecCtx->height;
	bool flag = true;
	SDL_Window* screen = nullptr;
	SDL_Thread *refresh_thread = SDL_CreateThread(&refDecVideo, NULL, NULL);//SDL线程进行事件通知更新帧，如此界面就不会卡住，可拖动鼠标了
	mySDLInstance = MySDL::getInstance();
	do{
	    SDL_WaitEvent(&m_event);
	    switch (m_event.type)
	    {
	    case REFRESH_EVENT://更新每一帧
	    {
		    while (av_read_frame(pFormatCtx, packet) >= 0) {
			    if (packet->stream_index == videoIndex) {

				    int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
				    if (ret < 0) {
					    printf("Decode Error.\n");
					    return -1;
				    }
				    if (got_picture) {
					//去除右侧的黑色的图像，和硬件相关
					sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
						pFrameYUV->data, pFrameYUV->linesize);
					printf("Decoded frame index: %d\n", frame_cnt);

					/*
					* 添加输出YUV的代码，取自于pFrameYUV，使用fwrite()
					*/
					ffmSendYUV2SDL(pFrameYUV->data[0], pFrameYUV->linesize[0], 0, 0, pCodecCtx->width
						, pCodecCtx->height, screen_w, screen_h, msec);
				 	frame_cnt++;
				    }
				    break;
			    }
			    else //非视频帧
			    {
			 	    continue;
			    }
		    }
		    break;
	    }
	    case SDL_WINDOWEVENT:
		//引用传出窗口大小的改变
		//SDL_GetWindowSize(screen, &screen_w, &screen_h);
		    break;
	    case SDL_QUIT:
		    threadExit = false;
		    break;
	    case BREAK_EVENT:
		    //8 退出SDL
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
int FfmpegDecodec::refDecVideo(void *) {//形参不能丢
	threadExit = true;
	while (threadExit) {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		if(startPushEvent)
		SDL_PushEvent(&event);
		SDL_Delay(delayTimemsec);//每隔40毫秒发送一个刷新事件
	}
	threadExit = true;
	//Break
	SDL_Event event;
	event.type = BREAK_EVENT;//发送退出信号
	SDL_PushEvent(&event);
	return 0;
}