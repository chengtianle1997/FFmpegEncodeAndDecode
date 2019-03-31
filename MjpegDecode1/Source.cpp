
//

//  main.cpp

//  OpenFile

//

//  Created by ination on 17/3/22.

//  Copyright © 2017年 ination. All rights reserved.

//



#include <iostream>

#ifdef __cplusplus

extern "C"

{

#endif

#include <libavformat/avformat.h>

#include <libavcodec/avcodec.h>

#include <libavutil/avutil.h>

#include <libswscale/swscale.h>

#ifdef __cplusplus

};

#endif







int main(int argc, const char * argv[]) {

	// insert code here...



	avcodec_register_all();

	av_register_all();

	avformat_network_init();



	//输出支持解封装格式

	//printf("======  av_input_format  =====\n");

	AVInputFormat *fmt = NULL;

	/*while ((fmt = av_iformat_next(fmt))) {

		printf("name : %s\n", fmt->name);

		printf("long_name : %s\n", fmt->long_name);

		printf("\n");

	}*/

	//printf("==============================\n");



	AVFormatContext *pAVFormatCtx = NULL;

	AVFormatContext *OutFormatCtx = NULL;

	

	pAVFormatCtx = avformat_alloc_context();

	OutFormatCtx = avformat_alloc_context();

	char filepath[] = "encode.mjpeg";



	//打开文件

	char errorBuf[1024];

	int retOpenFile = avformat_open_input(&pAVFormatCtx, filepath, NULL, NULL);

	if (0 != retOpenFile) {

		av_strerror(retOpenFile, errorBuf, sizeof(errorBuf));

		printf("Couldn't open file %s: %d(%s)\n", filepath, retOpenFile, errorBuf);

		return -1;

	}



	//输出文件信息

	printf("------------- File Information ------------------\n");

	av_dump_format(pAVFormatCtx, 0, filepath, 0);

	printf("-------------------------------------------------\n");



	//音视频分离

	int retFindStream = avformat_find_stream_info(pAVFormatCtx, NULL);

	if (0 != retFindStream) {

		av_strerror(retFindStream, errorBuf, sizeof(errorBuf));

		printf("Couldn't find stream %s: %d(%s)\n", filepath, retFindStream, errorBuf);

		return -1;

	}



	int videoStreamIndex = -1;

	for (int i = 0; i < pAVFormatCtx->nb_streams; i++) {

		AVStream *stream = pAVFormatCtx->streams[i];

		AVCodecParameters *codeParam = stream->codecpar;

		if (AVMEDIA_TYPE_VIDEO == codeParam->codec_type) {

			videoStreamIndex = i;

			break;

		}

	}

	if (-1 == videoStreamIndex) {

		printf("Didn't find a video stream.\n");

		return -1;

	}



	//视频流信息

	AVStream *videoStream = pAVFormatCtx->streams[videoStreamIndex];

	AVCodecParameters *codeParam = videoStream->codecpar;

	AVCodecContext *pAVCodeCtx = avcodec_alloc_context3(NULL);

	

	avcodec_parameters_to_context(pAVCodeCtx, codeParam);

	if (0 == pAVCodeCtx) {

		printf("Couldn't create AVCodecContext\n");

		return -1;

	}

	//输出流信息
	OutFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
	AVStream* video_st = avformat_new_stream(OutFormatCtx, 0);
	if (video_st == NULL)
	{
		printf("Output Stream Error!");
		return -1;
	}

	AVCodecContext *OutCodecCtx = video_st->codec;

	//查找视频解码器

	AVCodecID videoCodeId = codeParam->codec_id;

	AVCodec *videoDeCode = avcodec_find_decoder(videoCodeId);

	if (videoDeCode == NULL) {

		printf("Codec not found.\n");

		return -1;

	}

	AVCodec *ImageEncode = avcodec_find_encoder(videoCodeId);

	if (ImageEncode == NULL) {

		printf("Encode Codec not found.\n");

		return -1;

	}


	//打开视频解码器

	int retOpenVideoDecode = avcodec_open2(pAVCodeCtx, videoDeCode, NULL);

	if (retOpenVideoDecode != 0) {

		av_strerror(retOpenVideoDecode, errorBuf, sizeof(errorBuf));

		printf("open decode Error. %s\n", errorBuf);

		return -1;

	}

	//pAVCodeCtx->height = 

	//打开编码器

	if (avcodec_open2(OutCodecCtx, ImageEncode, NULL) < 0) {

		printf("Could not open Encode codec.");

		return -1;

	}


	avcodec_parameters_from_context(video_st->codecpar, OutCodecCtx);
	ImageEncode->capabilities = AV_CODEC_CAP_SLICE_THREADS;

	int i=0;//计数器

	bool bFirstFrame = false;

	AVPacket *avPacket = av_packet_alloc();

	AVFrame *avVideoFrame = av_frame_alloc();

	while (1) {

		AVPacket outPkt;

		char* out_file = 0;

		out_file = (char*)malloc(40);

		//输出文件名
		sprintf(out_file, "Image/Image%d.jpeg",  i);

		av_dump_format(pAVFormatCtx, 0, out_file, 1);

		//Output URL
		if (avio_open(&pAVFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
			printf("Couldn't open output file.");
			return -1;
		}

		avformat_init_output(pAVFormatCtx, 0);

		//Write Header

		avformat_write_header(pAVFormatCtx, 0);

		//从原始数据读取一帧

		av_read_frame(pAVFormatCtx, avPacket);

		avPacket->stream_index = video_st->index;

		//int ret = av_write_frame(pAVFormatCtx, avPacket);

		//av_interleaved_write_frame(pAVFormatCtx, avPacket);

		if (avPacket->stream_index == videoStreamIndex) {

			//送往解码器

			int retPackt = avcodec_send_packet(pAVCodeCtx, avPacket);

			if (retPackt < 0) {

				av_strerror(retPackt, errorBuf, sizeof(errorBuf));

				printf("packet Error. %s\n", errorBuf);

				continue;

			}

			//从解码器获取一帧

			int retDcode = avcodec_receive_frame(pAVCodeCtx, avVideoFrame);

			if (retDcode < 0) {

				av_strerror(retDcode, errorBuf, sizeof(errorBuf));

				printf("Decode Error. %s\n", errorBuf);

				continue;

			}
			else {
				int got_picture=0;

				int ret = avcodec_encode_video2(pAVCodeCtx, &outPkt, avVideoFrame, &got_picture);

				if (ret < 0) {

					printf("Encode Error.\n");

					return -1;

				}

				if (got_picture == 1) {

					outPkt.stream_index = video_st->index;

					ret = av_write_frame(pAVFormatCtx, &outPkt);

				}

				//avPacket->stream_index = video_st->index;

				//int ret = av_write_frame(pAVFormatCtx, avPacket);
				//bFirstFrame = true;

				//break;

			}
		}

			//Write Trailer
			av_write_trailer(pAVFormatCtx);

			i++;

			
			free(out_file);
			
	}



	//if (bFirstFrame) {

	//	//todo 图像处理

	//}



	//资源释放

	

	av_packet_free(&avPacket);

	av_frame_free(&avVideoFrame);

	avcodec_close(pAVCodeCtx);

	avformat_close_input(&pAVFormatCtx);

	avformat_network_deinit();

	printf("Everything is ok!");

	return 0;



}
