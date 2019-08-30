//
// Created by llm on 19-8-30.
//

#include "BaseChannel.h"

BaseChannel::BaseChannel(int id, AVCodecContext *codecContext) : id(id), codecContext(codecContext) {
    packets.setReleaseCallback(releaseAVPacket);
    frames.setReleaseCallback(releaseAVFrame);
}




