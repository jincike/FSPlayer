#include "stdafx.h"
#include "mySDL.h";

MySDL* MySDL::instance = nullptr;
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
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

	Uint32 pixformat = 0;
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat = SDL_PIXELFORMAT_IYUV;

	//4 创建纹理
	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);

	//5 创建播放显示位置的 矩形坐标：左上点+长宽
	SDL_Rect sdlRect;

	//unsigned char buffer[pixel_w*pixel_h*bpp / 8];
	unsigned char* buffer = new unsigned char[pixel_w*pixel_h*bpp / 8];
	while (1) {
		if (fread(buffer, 1, pixel_w*pixel_h*bpp / 8, fp) != pixel_w*pixel_h*bpp / 8) {
			// Loop
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
		//Delay 40ms
		SDL_Delay(msec);//控制播放速度

	}
	//8 退出SDL
	SDL_Quit();
	if (nullptr != buffer)
	{
		delete[]buffer;
		buffer = nullptr;
	}
	return 0;
}