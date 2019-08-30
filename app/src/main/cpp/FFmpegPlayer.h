//
// Created by llm on 19-8-29.
//

#ifndef NATIVEPLAYER_FFMPEGPLAYER_H
#define NATIVEPLAYER_FFMPEGPLAYER_H


#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "macro.h"
#include <pthread.h>

extern "C" {
#include <libavformat/avformat.h>
};

class FFmpegPlayer {
public:
    FFmpegPlayer(JavaCallHelper *javaCallHelper, char *dataSource);
    ~FFmpegPlayer();

    void prepare();

    void _prepare();

    void start();

    void _start();

    void setRenderCallback(RenderCallback renderCallback);

private:
    JavaCallHelper *javaCallHelper = 0;  // 回调java

    AudioChannel *audioChannel;
    VideoChannel *videoChannel;

    char *dataSource; //媒体文件路径

    pthread_t pid_prepare; //prepare线程id

    pthread_t pid_start; //start线程id

    AVFormatContext *formatContext = 0;

    int isPlaying = 0;

    RenderCallback renderCallback;
};

#endif //NATIVEPLAYER_FFMPEGPLAYER_H
