#include "FFmpegEncode.h"

int Encoder::InitJPEG(EncoderParam encoderparam)
{
	//Encode with FFmpeg
	in_w = encoderparam.in_w;

	in_h = encoderparam.in_h;

	av_register_all();

	pFormatCtx = avformat_alloc_context();

	//Guess format
	fmt = av_guess_format("mjpeg", NULL, NULL);

	pFormatCtx->oformat = fmt;

	video_st = avformat_new_stream(pFormatCtx, 0);
	if (video_st == NULL) {
		return -1;
	}

	pCodecCtx = video_st->codec;

	pCodecCtx->codec_id = fmt->video_codec;

	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;

	pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ444P;



	pCodecCtx->width = in_w;

	pCodecCtx->height = in_h;

	pCodecCtx->bit_rate = encoderparam.bitrate;

	pCodecCtx->bit_rate_tolerance = encoderparam.bitrate_tolerance;

	pCodecCtx->time_base.num = 1;

	pCodecCtx->time_base.den = 25;

	pCodecCtx->framerate.num = 25;

	pCodecCtx->framerate.den = 1;

	pCodecCtx->thread_count = encoderparam.ethread;

	av_dict_set(&param, "preset", "slow", 0);

	av_dict_set(&param, "tune", "zerolatecy", 0);


	//Output some information

	//av_dump_format(pFormatCtx, 0, out_file, 1);



	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);

	if (!pCodec) {

		printf("Codec not found.");

		return -1;

	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {

		printf("Could not open codec.");

		return -1;

	}

	pCodec->capabilities = AV_CODEC_CAP_SLICE_THREADS;

	avcodec_parameters_from_context(video_st->codecpar, pCodecCtx);


	picture = av_frame_alloc();

	size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

	picture->width = pCodecCtx->width;

	picture->height = pCodecCtx->height;

	picture->format = pCodecCtx->pix_fmt;

	picture_buf = (uint8_t *)av_malloc(size);

	if (!picture_buf)

	{

		return -1;

	}

	avpicture_fill((AVPicture *)picture, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

	//Write Header

	//avformat_write_header(pFormatCtx, NULL);

	y_size = pCodecCtx->width * pCodecCtx->height;

	return 0;
}

int Encoder::InitMJPEG(EncoderParam encoderparam)
{

	uint8_t *pDataForYUV = NULL;

	av_register_all();



	pCodec = avcodec_find_encoder(codec_id);
	if (!pCodec) {
		printf("Codec not found\n");
		return -1;
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		printf("Could not allocate video codec context\n");
		return -1;
	}

	/*hwconfig = avcodec_get_hw_config(pCodec, 0);
	if (!hwconfig) {
		printf("Could not get hardware config\n");
		return -1;
	}*/

	//hw_configs[0] = hwconfig;



	pCodecCtx->codec_id = AV_CODEC_ID_MJPEG;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->bit_rate = encoderparam.bitrate;
	pCodecCtx->bit_rate_tolerance = encoderparam.bitrate_tolerance;
	pCodecCtx->width = encoderparam.in_w;
	pCodecCtx->height = encoderparam.in_h;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;

	pCodecCtx->gop_size = 10;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ444P;
	pCodecCtx->thread_count = encoderparam.ethread;
	//pCodecCtx->slices = encoderparam.ethread;
	pCodecCtx->framerate.num = 25;
	pCodecCtx->framerate.den = 1;


	pCodec->capabilities = AV_CODEC_CAP_SLICE_THREADS;
	//pCodec->hw_configs = hw_configs;

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec\n");
		return -1;
	}

	picture = av_frame_alloc();
	if (!picture) {
		printf("Could not allocate video frame\n");
		return -1;
	}

	picture->width = pCodecCtx->width;

	picture->height = pCodecCtx->height;

	picture->format = pCodecCtx->pix_fmt;

	ret = av_image_alloc(picture->data, picture->linesize, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 16);
	if (ret < 0) {
		printf("Could not allocate raw picture buffer\n");
		return -1;
	}

	char* Filename;

	Filename = (char*)malloc(500);
	sprintf(Filename, "%s\\Camera_%s.mjpeg", encoderparam.filepath, encoderparam.SerialNum);

	//Output bitstream
	fp_out = fopen(Filename, "wb");
	if (!fp_out) {
		printf("Could not open %s\n", Filename);
		return -1;
	}

	uint8_t startcode[] = { 0,0,0xff,0xd8 };

	fwrite(startcode, 1, sizeof(startcode), fp_out);

	y_size = pCodecCtx->width * pCodecCtx->height;

	return 0;

}

int Encoder::EncodeJPEG(EncodeParam encodeparam)
{
	char* out_file = 0;

	out_file = (char*)malloc(40);

	//Êä³öÎÄ¼þ
	//sprintf(out_file, "%s\\Camera%d_%d.jpeg", encodeparam.filepath, encodeparam.CameraNum, encodeparam.pts);
	sprintf(out_file, "Camera%d_%d.jpeg", encodeparam.CameraNum, encodeparam.pts);
	//Output some information
	av_dump_format(pFormatCtx, 0, out_file, 1);

	//Output URL
	if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
		printf("Couldn't open output file.");
		return -1;
	}

	avformat_init_output(pFormatCtx, &param);

	//Write Header

	avformat_write_header(pFormatCtx, &param);

	//YUV420 Planar
	pDataForYUV = (uint8_t*)malloc(encodeparam.nWidth * encodeparam.nHeight * 3);
	if (NULL == pDataForYUV)
	{
		printf("malloc pDataForYUV420 fail !\n");
		//break;
	}

	av_new_packet(&pkt, y_size * 3);

	//printf("%d",sizeof(stOutFrame.pBufAddr));

	//Read YUV

	//memcpy(picture_buf, stOutFrame.pBufAddr, stOutFrame.stFrameInfo.nHeight*stOutFrame.stFrameInfo.nWidth);

	//picture->data[0] = picture_buf;              // Y

	//picture->data[1] = picture_buf + y_size;      // U 

	//picture->data[2] = picture_buf + y_size * 5 / 4;  // V


	for (int j = 0; j < encodeparam.nWidth * encodeparam.nHeight; j++)
	{
		pDataForYUV[j] = (uint8_t)encodeparam.pBufAddr[j];
		pDataForYUV[encodeparam.nWidth * encodeparam.nHeight + j] = 128;
		pDataForYUV[encodeparam.nWidth * encodeparam.nHeight * 2 + j] = 128;

	}

	picture->data[0] = pDataForYUV;

	picture->data[1] = pDataForYUV + y_size;

	picture->data[2] = pDataForYUV + y_size * 2;

	picture->pts = encodeparam.pts;

	//Encode

	ret = avcodec_encode_video2(pCodecCtx, &pkt, picture, &got_picture);

	if (ret < 0) {

		printf("Encode Error.\n");

		return -1;

	}

	if (got_picture == 1) {

		pkt.stream_index = video_st->index;

		ret = av_write_frame(pFormatCtx, &pkt);

	}

	//Write Trailer
	av_write_trailer(pFormatCtx);

	//i++;

	av_free_packet(&pkt);

	free(pDataForYUV);

	free(out_file);

	return 0;
}

int Encoder::EncodeMJPEG(EncodeParam encodeparam)
{
	pDataForYUV = (uint8_t*)malloc(encodeparam.nWidth * encodeparam.nHeight * 3);
	if (NULL == pDataForYUV)
	{
		printf("malloc pDataForYUV420 fail !\n");
	}

	for (int j = 0; j < encodeparam.nWidth * encodeparam.nHeight; j++)
	{
		pDataForYUV[j] = (uint8_t)encodeparam.pBufAddr[j];
		pDataForYUV[encodeparam.nWidth * encodeparam.nHeight + j] = 128;
		pDataForYUV[encodeparam.nWidth * encodeparam.nHeight * 2 + j] = 128;

	}

	av_init_packet(&pkt);

	pkt.data = NULL;    // packet data will be allocated by the encoder

	pkt.size = 0;

	//Read raw YUV data
	picture->data[0] = pDataForYUV;  //Y

	picture->data[1] = pDataForYUV + y_size;  //U

	picture->data[2] = pDataForYUV + y_size * 2;  //V

	picture->pts = encodeparam.pts;

	//Encode

	ret = avcodec_encode_video2(pCodecCtx, &pkt, picture, &got_picture);

	if (ret < 0) {

		printf("Encode Error.\n");

		return -1;

	}

	i++;

	if (got_picture) {
		//printf("Succeed to encode frame: %5d\tsize:%5d\n", framecount, pkt.size);
		//framecount++;
		fwrite(pkt.data, 1, pkt.size, fp_out);
		av_free_packet(&pkt);
	}

	av_free_packet(&pkt);

	free(pDataForYUV);

	return 0;
}

int Encoder::JPEGClean()
{
	printf("Encode Successful.\n");



	if (video_st) {

		avcodec_close(video_st->codec);

		av_free(picture);

		av_free(picture_buf);

	}

	avio_close(pFormatCtx->pb);

	avformat_free_context(pFormatCtx);

	return 0;

}

int Encoder::MJPEGClean()
{
	uint8_t endcode[] = { 0,0,0xff,0xd9 };

	fwrite(endcode, 1, sizeof(endcode), fp_out);

	fclose(fp_out);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	//av_freep(&picture->data[0]);
	av_frame_free(&picture);

	return 0;
}

int Encoder::MJPEGFlush()
{
	for (got_picture = 1; got_picture; i++) {
		ret = avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_picture);
		if (ret < 0) {
			printf("Error encoding frame\n");
			return -1;
		}
		if (got_picture) {
			printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", pkt.size);
			fwrite(pkt.data, 1, pkt.size, fp_out);
			av_free_packet(&pkt);
		}
	}

	return 0;
}

