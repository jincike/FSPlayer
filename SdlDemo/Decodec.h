#pragma once
#include "mySDL.h"
/**
* 简单的基于FFmpeg的解码器
* 本程序实现了视频文件的解码(支持HEVC，H.264，MPEG2等)。
*/
#include <stdio.h>
#include "iostream"

#define __STDC_CONSTANT_MACROS
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
};

class FfmpegDecodec
{
public:
	FfmpegDecodec();
	~FfmpegDecodec();
	static FfmpegDecodec* getInstance();

	//解封装
	int format2stream(AVFormatContext *pFormatCtx, char filepath[], int &videoIndex);
	//解码上下文
	int stream2frame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx);
	//解封装，写264文件
	int packet2H264(AVFormatContext *pFormatCtx, FILE *file, int vedioIndex);
	//解码packet，写到YUV文件
	int frame2YUV(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx
		, FILE *file, int videoIndex);
	//解码出YUV帧
	int frame2YUVEX(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx,int videoIndex,int msec);
	//将解码出的YVU纹理送到SDL中渲染播放
	void ffmSendYUV2SDL(void *v,int pitch,int x, int y, int pixel_w, int pixel_h
		, int screen_w, int screen_h, int msec);
	static int refDecVideo(void *);//静态成员函数才能使用函数指针
protected:
private:
	static FfmpegDecodec* instance;
	SDL_Event m_event;
	MySDL* mySDLInstance = nullptr;
	//内类来释放单例，防止内存泄漏
	class delInstance {
	public:
		~delInstance() {
			if (nullptr != instance)
			{
				delete instance;
				instance = nullptr;
			}
		}
	};
	static delInstance gc;
};