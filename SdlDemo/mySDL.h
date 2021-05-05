#pragma once
#include <mutex>
#define SDL_MAIN_HANDLED
extern "C"
{
#include "sdl/SDL.h" 
}

//Refresh Event
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
//Break
#define BREAK_EVENT  (SDL_USEREVENT + 2)

class MySDL
{
public:
	MySDL();
	~MySDL() {};
	static MySDL* getInstance();
	
	/**********
	*YUV���ݷ��͵�SDL�ⲥ��
	*fp[in]�ļ�
	*x,y��pixel_w��pixel_w��pixel_h :SDL_Rect����ʼ����ͳ���
	*screen_w,screen_h ���ڵĴ�С
	*ÿһ֡����ʱ  ���루Ĭ��40=1000/25֡��
	***********/
	int sendYUV2SDL(FILE* fp, int x, int y, int pixel_w, int pixel_h
		, int screen_w, int screen_h,int msec =25);
	//�ڴ���ֱ�ӽ�ÿһ֡YUV�͵�SDL����ת��Ӳ��
	int sendYUV2SDLDirect(void*data, int pitch, int x, int y, int pixel_w, int pixel_h
		, int screen_w, int screen_h, int msec);
	static int onSdlEvent(void *);
	static int refreshVideo(void *);//��̬��Ա��������ʹ�ú���ָ��
protected:
private:
	static MySDL* instance;
	
	bool firstFlag = true;
	SDL_Event m_event;
	SDL_Renderer* sdlRenderer = nullptr;
	SDL_Texture* sdlTexture = nullptr;
	SDL_Rect sdlRect;
	
	class gcInstance {
	public:
		~gcInstance()
		{
			if (nullptr!=instance)
			{
				delete instance;
				instance = nullptr;
			}
		}
	};
	static gcInstance gc;
};