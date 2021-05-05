#include "stdafx.h"
#include "mySDL.h";

//SDL相关的初始化
MySDL* MySDL::instance = nullptr;

std::mutex m_mutex;
static bool threadExit = true;
static bool startPushEvent = true;
static int delayTimemsec = 40;//25帧
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
	//多个地方监听事件会卡顿
	//SDL_Thread *onsdlEv_thread = SDL_CreateThread(&onSdlEvent, NULL, NULL);//线程捕捉SDL事件
}
int MySDL::sendYUV2SDL(FILE* fp,int x,int y,int pixel_w ,int pixel_h
	   ,int screen_w ,int screen_h ,int msec) {
	const int bpp = 12;//像素深度，每个像素所占比特
		
	//1 SDL初始化
	if (SDL_Init(SDL_INIT_VIDEO)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	//2 创建窗口
	//SDL 2.0 Support for multiple windows
	SDL_Window * screen = SDL_CreateWindow("FFmpeg SDL2 Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!screen) {
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}
	//3 创建渲染器
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RendererFlags::SDL_RENDERER_SOFTWARE);

	Uint32 pixformat = 0;
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat = SDL_PIXELFORMAT_IYUV;

	//4 创建纹理
	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);

	//5 创建播放显示位置的 矩形坐标：左上点+长宽
	SDL_Rect sdlRect;

	SDL_Thread *refresh_thread = SDL_CreateThread(&refreshVideo, NULL, NULL);//线程进行事件通知更新帧，如此界面就不会卡住，可拖动鼠标了
	//unsigned char buffer[pixel_w*pixel_h*bpp / 8];
	unsigned char* buffer = new unsigned char[pixel_w*pixel_h*bpp / 8];
	bool flag = true;
	do{
		SDL_WaitEvent(&m_event);
		switch (m_event.type)
		{
		case REFRESH_EVENT://更新每一帧
		{
			//Y的是宽*高，U\V分别是宽*高/4，即（宽*高）*（1+1/2）
			if (fread(buffer, 1, pixel_w*pixel_h*bpp / 8, fp) != pixel_w*pixel_h*bpp / 8) {
				// 读取最后一帧
				fseek(fp, 0, SEEK_SET);
				fread(buffer, 1, pixel_w*pixel_h*bpp / 8, fp);
			}
			//6 更新buffer里的YUV到纹理
			SDL_UpdateTexture(sdlTexture, NULL, buffer, pixel_w);

			sdlRect.x = 0;
			sdlRect.y = 0;
			sdlRect.w = screen_w;
			sdlRect.h = screen_h;

			//7 清空渲染器，拷贝纹理到渲染器，渲染器播放出来
			SDL_RenderClear(sdlRenderer);
			SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
			SDL_RenderPresent(sdlRenderer);
			//SDL_Delay(msec);//控制播放速度交给了线程
		break;
		}
		case SDL_WINDOWEVENT:
			//引用传出窗口大小的改变
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
	//8 退出SDL
	SDL_Quit();
	if (nullptr != buffer)
	{
		delete[]buffer;
		buffer = nullptr;
	}
	return 0;
}
//FFmpeg直接将YUV码流发送到SDL中播放
int MySDL::sendYUV2SDLDirect(void*data,int pitch,int x, int y, int pixel_w, int pixel_h
	, int screen_width, int screen_height, int msec) {
	while (firstFlag) {
		screen_w = screen_width;
		screen_h = screen_height;
		//1 SDL初始化
		if (SDL_Init(SDL_INIT_VIDEO)) {
			printf("Could not initialize SDL - %s\n", SDL_GetError());
			return -1;
		}
		//2 创建窗口
		//SDL 2.0 Support for multiple windows
		screen = SDL_CreateWindow("FFmpeg SDL2 Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (!screen) {
			printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
			return -1;
		}
		//3 创建渲染器
		sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RendererFlags::SDL_RENDERER_SOFTWARE);
		Uint32 pixformat = 0;
		//IYUV: Y + U + V  (3 planes)
		//YV12: Y + V + U  (3 planes)
		pixformat = SDL_PIXELFORMAT_IYUV;

		//4 创建纹理
		sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);

		//5 创建播放显示位置的 矩形坐标：左上点+长宽
		//SDL_Rect sdlRect;
		firstFlag = false;
	}
	//6 更新buffer里的YUV到纹理
	SDL_UpdateTexture(sdlTexture, NULL, data, pitch);

	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;

	//7 清空渲染器，拷贝纹理到渲染器，渲染器播放出来
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
	SDL_RenderPresent(sdlRenderer);
	//SDL_Delay(msec);//控制播放速度交给了线
	return 0;
}
//播放器未使用
int MySDL::refreshVideo(void *){//形参不能丢
	threadExit = true;
	while (threadExit) {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		if (startPushEvent) {
			SDL_PushEvent(&event);
		}
		SDL_Delay(delayTimemsec);//每隔40毫秒发送一个刷新事件
	}
	threadExit = true;
	//Break
	SDL_Event event;
	event.type = BREAK_EVENT;//发送退出信号
	SDL_PushEvent(&event);
	return 0;
}