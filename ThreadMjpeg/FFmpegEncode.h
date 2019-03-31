#pragma once

#include "MvCameraControl.h"
extern "C"

{

#include "libavcodec/avcodec.h"

#include "libavformat/avformat.h"

#include "libavformat/avio.h"

#include "libavutil/opt.h"

#include "libavutil/imgutils.h"

};

typedef struct {
	//YUV's width and height
	int in_w;
	int in_h;

	//BitRate
	int bitrate = 200000000;
	int bitrate_tolerance = 8000000;

	//EncoderTHread
	int ethread = 1;

	//CameraNum
	int CameraNum = 0;
	unsigned char SerialNum[INFO_MAX_BUFFER_SIZE];
	const char* filepath;

}EncoderParam;

typedef struct {
	//YUV's width and height
	int nWidth;
	int nHeight;

	//ImageBuffer
	unsigned char* pBufAddr;

	//Timestamp
	int pts;
	//CameraNum
	int CameraNum = 0;

	//FrameCut
	int FrameCut = 5;//Encode 1 frame before every FrameCut frame

	const char* filepath;

}EncodeParam;

class Encoder {
public:
	//Encode Initiate
	int InitJPEG(EncoderParam encoderparam);
	int InitMJPEG(EncoderParam encoderparam);

	//Encode Image
	int EncodeJPEG(EncodeParam encodeparam);

	//Encode Video
	int EncodeMJPEG(EncodeParam encodeparam);

	//Clean Context
	int JPEGClean();
	int MJPEGClean();

	//Flush Encoder
	int MJPEGFlush(); //to be continued

private:
	AVFormatContext* pFormatCtx;

	AVOutputFormat* fmt;

	AVStream* video_st;

	AVCodecContext* pCodecCtx;

	AVCodec* pCodec;

	uint8_t* picture_buf;

	AVFrame* picture;

	AVPacket pkt;

	uint8_t *pDataForYUV = NULL;

	AVCodecID codec_id = AV_CODEC_ID_MJPEG;

	AVDictionary *param = 0;

	const AVCodecHWConfig *hwconfig;

	//const AVCodecHWConfigInternal **hw_configs;

	int y_size;

	int got_picture = 0;

	int size;

	//int framecount = 100;

	int ret = 0;

	int i = 0;

	//FILE *in_file = NULL;                            //YUV source

	int in_w, in_h;       //YUV's width and height

	//int CameraNum = 0;

	FILE *fp_out = NULL;
};