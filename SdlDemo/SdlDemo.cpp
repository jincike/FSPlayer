// SdlDemo.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <iostream>
#include "mySDL.h"

class MySDL;
int main()
{
	MySDL* mySDLInstance = MySDL::getInstance();
	//�������ں�Դ�ļ��Ĵ�С
	int screen_w = 640, screen_h = 360;
	const int pixel_w = 640, pixel_h = 360;
	int msec = 40;
	//��YUV�ļ�
	FILE *fp = nullptr;
	errno_t fpRet = fopen_s(&fp,"sintel_640_360.yuv", "rb+");
	if (fpRet != 0) {
	    printf("cannot open this file\n");
	    return -1;
	}
	int Ret = mySDLInstance->sendYUV2SDL(fp,0,0,pixel_w,pixel_h,screen_w,screen_h, msec);
    return 0;
}

