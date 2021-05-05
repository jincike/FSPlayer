#include "stdafx.h"
#include "mySDL.h";

//SDL��صĳ�ʼ��
MySDL* MySDL::instance = nullptr;

std::mutex m_mutex;
static bool threadExit = true;
static bool startPushEvent = true;
static int delayTimemsec = 40;//25֡
static SDL_Window * screen = nullptr;
static int screen_w = 40, screen_h = 30;
MySDL* MySDL::getInstance() {
	std::lock_guard<std::mutex>lkg(m_mutex);
    if (nullptr==instance)
    {
		instance = new MySDL;
    }
	return instance;
}
MySDL::MySDL() 
{
	//����ط������¼��Ῠ��
	//SDL_Thread *onsdlEv_thread = SDL_CreateThread(&onSdlEvent, NULL, NULL);//�̲߳�׽SDL�¼�
}
int MySDL::sendYUV2SDL(FILE* fp,int x,int y,int pixel_w ,int pixel_h
	   ,int screen_w ,int screen_h ,int msec) {
	const int bpp = 12;//������ȣ�ÿ��������ռ����
		
	//1 SDL��ʼ��
	if (SDL_Init(SDL_INIT_VIDEO)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	//2 ��������
	//SDL 2.0 Support for multiple windows
	SDL_Window * screen = SDL_CreateWindow("FFmpeg SDL2 Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!screen) {
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}
	//3 ������Ⱦ��
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RendererFlags::SDL_RENDERER_SOFTWARE);

	Uint32 pixformat = 0;
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat = SDL_PIXELFORMAT_IYUV;

	//4 ��������
	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);

	//5 ����������ʾλ�õ� �������꣺���ϵ�+����
	SDL_Rect sdlRect;

	SDL_Thread *refresh_thread = SDL_CreateThread(&refreshVideo, NULL, NULL);//�߳̽����¼�֪ͨ����֡����˽���Ͳ��Ῠס�����϶������
	//unsigned char buffer[pixel_w*pixel_h*bpp / 8];
	unsigned char* buffer = new unsigned char[pixel_w*pixel_h*bpp / 8];
	bool flag = true;
	do{
		SDL_WaitEvent(&m_event);
		switch (m_event.type)
		{
		case REFRESH_EVENT://����ÿһ֡
		{
			//Y���ǿ�*�ߣ�U\V�ֱ��ǿ�*��/4��������*�ߣ�*��1+1/2��
			if (fread(buffer, 1, pixel_w*pixel_h*bpp / 8, fp) != pixel_w*pixel_h*bpp / 8) {
				// ��ȡ���һ֡
				fseek(fp, 0, SEEK_SET);
				fread(buffer, 1, pixel_w*pixel_h*bpp / 8, fp);
			}
			//6 ����buffer���YUV������
			SDL_UpdateTexture(sdlTexture, NULL, buffer, pixel_w);

			sdlRect.x = 0;
			sdlRect.y = 0;
			sdlRect.w = screen_w;
			sdlRect.h = screen_h;

			//7 �����Ⱦ��������������Ⱦ������Ⱦ�����ų���
			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);
			//SDL_Delay(msec);//���Ʋ����ٶȽ������߳�
		break;
		}
		case SDL_WINDOWEVENT:
			//���ô������ڴ�С�ĸı�
			SDL_GetWindowSize(screen, &screen_w, &screen_h);
			break;
		case SDL_QUIT:
			threadExit = false;
			break;
		case BREAK_EVENT:
			flag = false;
			break;
		case SDL_KEYUP:
		{
			switch (m_event.key.keysym.sym)
			{
			case SDLK_SPACE:
				startPushEvent = !startPushEvent;
				break;
			case SDLK_ESCAPE:
				flag = false;
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
	} while (flag);
	//8 �˳�SDL
	SDL_Quit();
	if (nullptr != buffer)
	{
		delete[]buffer;
		buffer = nullptr;
	}
	return 0;
}
//FFmpegֱ�ӽ�YUV�������͵�SDL�в���
int MySDL::sendYUV2SDLDirect(void*data,int pitch,int x, int y, int pixel_w, int pixel_h
	, int screen_width, int screen_height, int msec) {
	while (firstFlag) {
		screen_w = screen_width;
		screen_h = screen_height;
		//1 SDL��ʼ��
		if (SDL_Init(SDL_INIT_VIDEO)) {
			printf("Could not initialize SDL - %s\n", SDL_GetError());
			return -1;
		}
		//2 ��������
		//SDL 2.0 Support for multiple windows
		screen = SDL_CreateWindow("FFmpeg SDL2 Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (!screen) {
			printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
			return -1;
		}
		//3 ������Ⱦ��
		sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RendererFlags::SDL_RENDERER_SOFTWARE);
		Uint32 pixformat = 0;
		//IYUV: Y + U + V  (3 planes)
		//YV12: Y + V + U  (3 planes)
		pixformat = SDL_PIXELFORMAT_IYUV;

		//4 ��������
		sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);

		//5 ����������ʾλ�õ� �������꣺���ϵ�+����
		//SDL_Rect sdlRect;
		firstFlag = false;
	}
	//6 ����buffer���YUV������
	SDL_UpdateTexture(sdlTexture, NULL, data, pitch);

	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;

	//7 �����Ⱦ��������������Ⱦ������Ⱦ�����ų���
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
	SDL_RenderPresent(sdlRenderer);
	//SDL_Delay(msec);//���Ʋ����ٶȽ�������
	return 0;
}
//������δʹ��
int MySDL::refreshVideo(void *){//�ββ��ܶ�
	threadExit = true;
	while (threadExit) {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		if (startPushEvent) {
			SDL_PushEvent(&event);
		}
		SDL_Delay(delayTimemsec);//ÿ��40���뷢��һ��ˢ���¼�
	}
	threadExit = true;
	//Break
	SDL_Event event;
	event.type = BREAK_EVENT;//�����˳��ź�
	SDL_PushEvent(&event);
	return 0;
}