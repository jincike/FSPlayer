#pragma once
#include <mutex>
#define SDL_MAIN_HANDLED
extern "C"
{
#include "SDL.h" 
}
class MySDL
{
public:
	MySDL() {};
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
	
protected:
private:
	static MySDL* instance;
	
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