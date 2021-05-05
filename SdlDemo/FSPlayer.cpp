// SdlDemo.cpp : �������̨Ӧ�ó������ڵ㡣
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
//av_log��־
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
	//��ʼ��
	AVFormatContext	*pFormatCtx;
	int				videoIndex;//��Ƶ����indexcode
	AVCodecContext	*pCodecCtx;

	av_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_DEBUG);
	av_log_set_callback(FFmpegLogFunc);
	pFormatCtx = avformat_alloc_context();
	//��ȡ����������
	FfmpegDecodec* instanceDecodec = FfmpegDecodec::getInstance();

	char filepath[] = "Titanic.ts";//�����ļ�·��
	av_log(NULL, AV_LOG_INFO, "begin:%s", filepath);
	//1��ȡ��װ������
	instanceDecodec->format2stream(pFormatCtx, filepath, videoIndex);

	//3��ȡ����������
	pCodecCtx = pFormatCtx->streams[videoIndex]->codec;
	instanceDecodec->stream2frame(pFormatCtx, pCodecCtx);

	//4����yuv����һ���͵�SDL����Ⱦ����
	//�����ٶ�
	int msecRate = 40;
	int retDecodec = instanceDecodec->frame2YUVEX(pFormatCtx, pCodecCtx, videoIndex, msecRate);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);//��ȡ��ַ����������
	return 0;
}

