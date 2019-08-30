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

class BaseChannel {
public:
    BaseChannel(int id, AVCodecContext *codecContext);
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

    //对应的音频流或视频流index
    int id;

    AVCodecContext *codecContext;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;

    int isPlaying;
};


#endif //NATIVEPLAYER_BASECHANNEL_H
