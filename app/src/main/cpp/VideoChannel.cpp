//
// Created by llm on 19-8-29.
//

#include "VideoChannel.h"
#include "macro.h"

/**
 * 丢包（AVPacket）
 * @param q
 */
void dropAVPacket(queue<AVPacket *> &q) {
    while (!q.empty()) {
        AVPacket *avPacket = q.front();
        // I 帧、B 帧、 P 帧
        //不能丢 I 帧,AV_PKT_FLAG_KEY: I帧（关键帧）
        if (avPacket->flags != AV_PKT_FLAG_KEY) {
            //丢弃非 I 帧
            BaseChannel::releaseAVPacket(&avPacket);
            q.pop();
        } else {
            break;
        }
    }
}

/**
 * 丢包（AVFrame）
 * @param q
 */
void dropAVFrame(queue<AVFrame *> &q) {
    if (!q.empty()) {
        AVFrame *avFrame = q.front();
        BaseChannel::releaseAVFrame(&avFrame);
        q.pop();
    }
}

VideoChannel::VideoChannel(int id, AVCodecContext *codecContext, int fps,
        AVRational time_base, JavaCallHelper *javaCallHelper )
        : BaseChannel(id, codecContext, time_base, javaCallHelper){
    this->fps = fps;
    packets.setSyncHandle(dropAVPacket);
    frames.setSyncHandle(dropAVFrame);
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
    isPlaying = 0;
    javaCallHelper = 0;
    packets.setWork(0);
    frames.setWork(0);
    pthread_join(pid_video_decode, 0);
    pthread_join(pid_video_play, 0);
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

    double extra_delay;
    double real_delay;
    double audio_time;
    double video_time;
    double time_diff;

    int64_t running_time;
    int64_t clock_time_diff;

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
        extra_delay = frame->repeat_pict / (2 * fps);
        real_delay = delay_time_per_frame + extra_delay;
        //单位是：微秒
        video_time = frame->best_effort_timestamp * av_q2d(time_base);
        LOGE("vidoe time: %lf, %ld", video_time,  frame->best_effort_timestamp);
        LOGE("av_gettime(): %ld" , av_gettime());

        if (clockTime) {
            if (frame->best_effort_timestamp == 0) {
                int64_t basetime = clockTime->getClockTime();
                clockTime->setClockBasetime(basetime);
            } else {
                running_time = clockTime->getClockTime() - clockTime->getClockBasetime();
                clock_time_diff = (int64_t)(video_time * 1000000) - running_time;

                LOGE("clock_time_diff: %ld" , clock_time_diff);
                if (clock_time_diff > 0) {
                    if (clock_time_diff > 1000000) {
                        av_usleep((real_delay * 2) * 1000000);
                    } else {
                        av_usleep(((real_delay * 1000000) + clock_time_diff));
                    }
                } else if (clock_time_diff < 0) {
                    if (abs(clock_time_diff) >= 50000) {
                        frames.sync();
                        continue;
                    }
                }
            }
        } else if (!audioChannel) {
            av_usleep(real_delay * 1000000);

            if (javaCallHelper) {
                javaCallHelper->onProgress(THREAD_CHILD, video_time);
            }
        } else {
            audio_time = audioChannel->audio_time;
            time_diff = video_time - audio_time;
            if (time_diff > 0) {
           //     LOGE("视频比音频快：%lf", time_diff);

                if (time_diff > 1) {
                    av_usleep((real_delay * 2) * 1000000);
                } else {
                    av_usleep((real_delay + time_diff) * 1000000);
                }
            } else if (time_diff < 0) {
            //    LOGE("音频比视频快: %lf", fabs(time_diff));
                //音频比视频快：追音频（尝试丢视频包）
                //视频包：packets 和 frames
                if (fabs(time_diff) >= 0.05) {
                    //时间差如果大于0.05，有明显的延迟感
                    //丢包：要操作队列中数据！一定要小心！
//                    packets.sync();
                    frames.sync();
                    continue;
                }
            }
        }

        renderCallback(dst_data[0], dst_linesize[0], codecContext->width, codecContext->height);

        releaseAVFrame(&frame);
    }

    releaseAVFrame(&frame);

    isPlaying = 0;

    av_free(dst_data[0]);
    sws_freeContext(sws_ctx);
}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
}

