

#pragma once
#ifndef  _VID_FFMPEG_H
#define _VID_FFMPEG_H
#endif // ! _VID_FFMPEG_H



extern "C" {
#include "inttypes.h"    
#include "libavcodec/avcodec.h"    
#include "libavformat/avformat.h"    
#include "libavfilter/avfilter.h"    
#include "libavfilter/buffersink.h"    
#include "libavfilter/buffersrc.h"    
#include "libavutil/avutil.h"    
#include "libswscale/swscale.h"  
#include "libswresample/swresample.h"  
#include "libavutil/avassert.h"  
#include "libavutil/channel_layout.h"  
#include "libavutil/opt.h"  
#include "libavutil/mathematics.h"  
#include "libavutil/timestamp.h"  
};
#include<opencv2/opencv.hpp>
using namespace cv;

typedef struct {
	const char *input_file_name;
	const char *output_file_name;
	int frame_width;
	int frame_height;
} IOParam;

typedef struct {
	AVStream *st;
	int64_t next_pts;
	int samples_count;
	AVFrame *frame;
	AVFrame *tmp_frame;

	float t, tincr, tincr2;
	struct SwsContext *sws_ctx;
	struct SwrContext *swr_ctx;
} OutputStream;

class  VideoWriterFFmpeg
{
public:
	VideoWriterFFmpeg();
	~VideoWriterFFmpeg();
	//file_name ,bitrate,frame_rate,width,height
	int open(char *, int, int, int, int);
	int write(Mat);
	void flush();
	int add_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, \
    	enum AVCodecID codec_id, int bitrate, int frame_rate, int width, int height);
    int fill_yuv_image(AVFrame *pict, int width, int height, Mat src);
private:
	IOParam io;
	int stream_frame_rate;
	int frame_rate;
	int bitrate;
	int videoFrameIdx;
	OutputStream video_st;
	AVOutputFormat *fmt;
	AVFormatContext *oc;
	AVCodec *video_codec;
};

