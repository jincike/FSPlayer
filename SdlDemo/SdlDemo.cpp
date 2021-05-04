// SdlDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "mySDL.h"

class MySDL;
int main()
{
	MySDL* mySDLInstance = MySDL::getInstance();
	//创建窗口和源文件的大小
	int screen_w = 640, screen_h = 360;
	const int pixel_w = 640, pixel_h = 360;
	int msec = 40;
	//打开YUV文件
	FILE *fp = nullptr;
	errno_t fpRet = fopen_s(&fp,"sintel_640_360.yuv", "rb+");
	if (fpRet != 0) {
	    printf("cannot open this file\n");
	    return -1;
	}
	int Ret = mySDLInstance->sendYUV2SDL(fp,0,0,pixel_w,pixel_h,screen_w,screen_h, msec);
    return 0;
}

