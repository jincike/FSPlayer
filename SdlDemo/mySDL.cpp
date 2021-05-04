#include "stdafx.h"
#include "mySDL.h";

MySDL* MySDL::instance = nullptr;
static bool threadExit = true;
static bool startPushEvent = true;
std::mutex m_mutex;

MySDL* MySDL::getInstance() {
	std::lock_guard<std::mutex>lkg(m_mutex);
    if (nullptr==instance)
    {
		instance = new MySDL;
    }
	return instance;
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
			}
			break;
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
				if (m_event.key.keysym.sym == SDLK_SPACE) {
					startPushEvent = !startPushEvent;
				}
				else if (m_event.key.keysym.sym == SDLK_ESCAPE) {
					SDL_Log("ESC is pressed , it will exit \n");
					break;
				}
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
int MySDL::refreshVideo(void *){//�ββ��ܶ�
	threadExit = true;
	while (threadExit) {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		if (startPushEvent) {
			SDL_PushEvent(&event);
		}
		SDL_Delay(40);//ÿ��40���뷢��һ��ˢ���¼�
	}
	threadExit = true;
	//Break
	SDL_Event event;
	event.type = BREAK_EVENT;//�����˳��ź�
	SDL_PushEvent(&event);
	return 0;
}