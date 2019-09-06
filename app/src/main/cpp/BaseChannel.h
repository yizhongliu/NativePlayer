//
// Created by llm on 19-8-29.
//

#ifndef NATIVEPLAYER_BASECHANNEL_H
#define NATIVEPLAYER_BASECHANNEL_H


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
};

#include "safe_queue.h"
#include "JavaCallHelper.h"
#include "net/PlayClockTime.h"

class BaseChannel {
public:
    BaseChannel(int id, AVCodecContext *codecContext, AVRational time_base, JavaCallHelper *javaCallHelper);
    virtual ~BaseChannel() {
        packets.clear();
        frames.clear();
    }

    /**
 * 释放 AVPacket
 * @param packet
 */
    static void releaseAVPacket(AVPacket **packet) {
        if (packet) {
            av_packet_free(packet);
            *packet = 0;
        }
    }

    /**
     * 释放 AVFrame
     * @param frame
     */
    static void releaseAVFrame(AVFrame **frame) {
        if (frame) {
            av_frame_free(frame);
            *frame = 0;
        }
    }

    virtual void start() = 0;

    virtual void stop() = 0;

    void setClockTime(PlayClockTime *clockTime);

    //对应的音频流或视频流index
    int id;

    AVCodecContext *codecContext;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;

    AVRational time_base;
    double audio_time;

    bool isPlaying = false;
    JavaCallHelper *javaCallHelper = 0;

    PlayClockTime * clockTime = 0;
};


#endif //NATIVEPLAYER_BASECHANNEL_H
