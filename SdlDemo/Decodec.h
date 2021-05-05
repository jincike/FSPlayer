#pragma once
#include "mySDL.h"
/**
* �򵥵Ļ���FFmpeg�Ľ�����
* ������ʵ������Ƶ�ļ��Ľ���(֧��HEVC��H.264��MPEG2��)��
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

	//���װ
	int format2stream(AVFormatContext *pFormatCtx, char filepath[], int &videoIndex);
	//����������
	int stream2frame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx);
	//���װ��д264�ļ�
	int packet2H264(AVFormatContext *pFormatCtx, FILE *file, int vedioIndex);
	//����packet��д��YUV�ļ�
	int frame2YUV(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx
		, FILE *file, int videoIndex);
	//�����YUV֡
	int frame2YUVEX(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx,int videoIndex,int msec);
	//���������YVU�����͵�SDL����Ⱦ����
	void ffmSendYUV2SDL(void *v,int pitch,int x, int y, int pixel_w, int pixel_h
		, int screen_w, int screen_h, int msec);
	static int refDecVideo(void *);//��̬��Ա��������ʹ�ú���ָ��
protected:
private:
	static FfmpegDecodec* instance;
	SDL_Event m_event;
	MySDL* mySDLInstance = nullptr;
	//�������ͷŵ�������ֹ�ڴ�й©
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