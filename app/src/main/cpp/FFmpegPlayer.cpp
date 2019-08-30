//
// Created by llm on 19-8-29.
//

#include "FFmpegPlayer.h"

FFmpegPlayer::FFmpegPlayer(JavaCallHelper *javaCallHelper, char *dataSource) {
    //参数robust放 jni 层判断
    this->javaCallHelper = javaCallHelper;

    this->dataSource = new char[strlen(dataSource) + 1];
    strcpy(this->dataSource, dataSource);
}

FFmpegPlayer::~FFmpegPlayer() {
    DELETE(javaCallHelper);
    DELETE(dataSource);
}

void *task_prepare(void *args) {
    FFmpegPlayer *ffmpegPlayer = static_cast<FFmpegPlayer *>(args);
    ffmpegPlayer->_prepare();

    return 0;//一定一定一定要返回0！！！
}

void FFmpegPlayer::prepare() {
    pthread_create(&pid_prepare, 0, task_prepare, this);
}

void FFmpegPlayer::_prepare() {

    formatContext = avformat_alloc_context();

    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary, "timeout", "10000000", 0); //超时时间 10 秒

    int ret = avformat_open_input(&formatContext, dataSource, 0, &dictionary);

    av_dict_free(&dictionary);

    if (ret != 0) {
        LOGE("fail to open media : %s", av_err2str(ret));

        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        }

        return;
    }

    ret = avformat_find_stream_info(formatContext, 0);
    if (ret < 0) {
        LOGE("avformat_find_stream_info error:%s", av_err2str(ret));

        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; i++) {
        AVStream *stream = formatContext->streams[i];

        AVCodecParameters *codecParameters = stream->codecpar;

        AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            LOGE("avcodec_find_decoder error");
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            }

            return;
        }

        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            LOGE("avcodec_alloc_context3");
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
        }

        ret = avcodec_parameters_to_context(codecContext, codecParameters);
        if (ret < 0) {
            LOGE("avcodec_parameters_to_context error:%s", av_err2str(ret));
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }

        ret = avcodec_open2(codecContext, codec, 0);
        if (ret != 0) {
            LOGE("avcodec_open2 error:%s", av_err2str(ret));
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            }
        }

        if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(i, codecContext);
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            AVRational frame_rate = stream->avg_frame_rate;
            int fps = av_q2d(frame_rate);

            videoChannel = new VideoChannel(i, codecContext, fps);
            videoChannel->setRenderCallback(renderCallback);
        }
    }

    if (!audioChannel && !videoChannel) {
        LOGE("没有音视频");
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        }
        return;
    }
    //准备好了，反射通知java
    if (javaCallHelper) {
        javaCallHelper->onPrepared(THREAD_CHILD);
    }
}

void *task_start(void *args) {
    FFmpegPlayer *ffmpegPlayer = static_cast<FFmpegPlayer *>(args);
    ffmpegPlayer->_start();

    return 0;//一定一定一定要返回0！！！
}

void FFmpegPlayer::start() {
    isPlaying = 1;
    if (videoChannel) {
        videoChannel->start();
    }

    if (audioChannel) {
        audioChannel->start();
    }

    pthread_create(&pid_start, 0, task_start, this);
}

void FFmpegPlayer::_start() {
    int ret;
    while (isPlaying) {
        if (videoChannel && videoChannel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }

        AVPacket *packet = av_packet_alloc();

        ret = av_read_frame(formatContext, packet);
        if (ret == 0) {
            if (videoChannel && packet->stream_index == videoChannel->id) {
                videoChannel->packets.push(packet);
            } else if (audioChannel && packet->stream_index == audioChannel->id) {
              //  audioChannel->packets.push(packet);
              av_packet_free(&packet);
            }
        } else if (ret == AVERROR_EOF) {

        } else {
            LOGE("av_read_frame error:%s", av_err2str(ret));
            av_packet_free(&packet);
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_READ_PACKETS_FAIL);
            }
            break;
        }
    }

    isPlaying = 0;

    videoChannel->stop();
    audioChannel->stop();
}

void FFmpegPlayer::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}