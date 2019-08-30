//
// Created by llm on 19-8-29.
//

#ifndef NATIVEPLAYER_AUDIOCHANNEL_H
#define NATIVEPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int id, AVCodecContext *avCodecContext);
    ~AudioChannel();

    void start();

    void stop();

};

#endif //NATIVEPLAYER_AUDIOCHANNEL_H
