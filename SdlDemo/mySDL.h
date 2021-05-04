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
	*YUV数据发送到SDL库播放
	*fp[in]文件
	*x,y，pixel_w，pixel_w，pixel_h :SDL_Rect的起始坐标和长宽
	*screen_w,screen_h 窗口的大小
	*每一帧的延时  毫秒（默认40=1000/25帧）
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