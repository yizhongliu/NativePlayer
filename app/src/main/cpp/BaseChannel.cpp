//
// Created by llm on 19-8-30.
//

#include "BaseChannel.h"

BaseChannel::BaseChannel(int id, AVCodecContext *codecContext, AVRational time_base, JavaCallHelper *javaCallHelper) : id(id),
                                                                                                                       codecContext(codecContext),
                                                                                                                       time_base(time_base),
                                                                                                                       javaCallHelper(javaCallHelper) {
    packets.setReleaseCallback(releaseAVPacket);
    frames.setReleaseCallback(releaseAVFrame);
}

void BaseChannel::setClockTime(PlayClockTime *clockTime) {
    this->clockTime = clockTime;
}




