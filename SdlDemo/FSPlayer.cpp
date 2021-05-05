// SdlDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "mySDL.h"
#include "Decodec.h"

#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
};
#define RETSUCESS 0

class FfmpegDecodec;
class MySDL;
//av_log日志
void FFmpegLogFunc(void* ptr, int level, const char * fmt, va_list vl) {
	FILE *fp = nullptr;
	int ret = fopen_s(&fp, "FSPlayerLog.txt", "a+");
	if (0 == ret) {
		vfprintf(fp, fmt, vl);
	}
	fflush(fp);
	fclose(fp);
}

int main()
{
	//初始化
	AVFormatContext	*pFormatCtx;
	int				videoIndex;//视频流的indexcode
	AVCodecContext	*pCodecCtx;

	av_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_DEBUG);
	av_log_set_callback(FFmpegLogFunc);
	pFormatCtx = avformat_alloc_context();
	//获取解码器单例
	FfmpegDecodec* instanceDecodec = FfmpegDecodec::getInstance();

	char filepath[] = "Titanic.ts";//输入文件路径
	av_log(NULL, AV_LOG_INFO, "begin:%s", filepath);
	//1获取封装上下文
	instanceDecodec->format2stream(pFormatCtx, filepath, videoIndex);

	//3获取解码上下文
	pCodecCtx = pFormatCtx->streams[videoIndex]->codec;
	instanceDecodec->stream2frame(pFormatCtx, pCodecCtx);

	//4解码yuv，进一步送到SDL中渲染播放
	//播放速度
	int msecRate = 40;
	int retDecodec = instanceDecodec->frame2YUVEX(pFormatCtx, pCodecCtx, videoIndex, msecRate);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);//获取地址，不是引用
	return 0;
}

