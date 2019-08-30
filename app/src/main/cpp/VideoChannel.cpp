//
// Created by llm on 19-8-29.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel(int id, AVCodecContext *codecContext, int fps) : BaseChannel(id, codecContext){
    this->fps = fps;
}

VideoChannel::~VideoChannel() {

}

void *task_video_decode(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);

    videoChannel->video_decode();
    return 0; //???
}

void *task_video_play(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);

    videoChannel->video_play();
    return 0; //???
}

void VideoChannel::start() {
    isPlaying = 1;

    packets.setWork(1);
    frames.setWork(1);

    pthread_create(&pid_video_decode, 0, task_video_decode, this);

    pthread_create(&pid_video_play, 0, task_video_play, this);

}

void VideoChannel::stop() {

}

void VideoChannel::video_decode() {
    AVPacket *avPacket;
    int ret;
    while (isPlaying) {
        ret = packets.pop(avPacket);
        //如果停止播放，则跳出循环，不继续执行，在循环外释放packet
        if (!isPlaying) {
            break;
        }

        if (!ret) {
            continue;
        }

        ret = avcodec_send_packet(codecContext, avPacket);
        if (ret != 0) {
            break;
        }

        releaseAVPacket(&avPacket);

        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            releaseAVFrame(&frame);
            continue;
        } else if (ret != 0) {
            releaseAVFrame(&frame);
            break;
        }

        while (isPlaying && frames.size() > 100) {
            av_usleep(10* 1000);
            continue;
        }

        frames.push(frame);

    }

    releaseAVPacket(&avPacket);
}

void VideoChannel::video_play() {
    AVFrame *frame = 0;

    uint8_t *dst_data[4];
    int dst_linesize[4];
    SwsContext *sws_ctx = sws_getContext(codecContext->width,
                                         codecContext->height,
                                         codecContext->pix_fmt,
                                         codecContext->width,
                                         codecContext->height,
                                         AV_PIX_FMT_RGBA,
                                         SWS_BILINEAR, NULL, NULL, NULL);

    av_image_alloc(dst_data, dst_linesize,
                   codecContext->width, codecContext->height, AV_PIX_FMT_RGBA, 1);

    double delay_time_per_frame = 1.0 / fps;

    int ret;
    while (isPlaying) {
        ret = frames.pop(frame);
        if (!isPlaying) {
            //如果停止播放了，跳出循环 释放packet
            break;
        }
        if (!ret) {
            //取数据包失败
            continue;
        }

        sws_scale(sws_ctx, frame->data, frame->linesize, 0, codecContext->height, dst_data, dst_linesize);

        //extra_delay = repeat_pict / (2*fps)
        double extra_delay = frame->repeat_pict / (2 * fps);
        double real_delay = delay_time_per_frame + extra_delay;
        //单位是：微秒
        av_usleep(real_delay * 1000000);

        renderCallback(dst_data[0], dst_linesize[0], codecContext->width, codecContext->height);

        releaseAVFrame(&frame);
    }

    releaseAVFrame(&frame);

    isPlaying = 0;

    av_free(&dst_data[0]);
    sws_freeContext(sws_ctx);
}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}

