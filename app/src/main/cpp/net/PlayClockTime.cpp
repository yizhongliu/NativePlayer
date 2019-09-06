//
// Created by llm on 19-9-5.
//

#include <linux/time.h>
#include <sys/time.h>
#include "PlayClockTime.h"

PlayClockTime::PlayClockTime() {
    basetime = 0;
}

PlayClockTime::~PlayClockTime() {
}

void PlayClockTime::setClockBasetime(int64_t basetime) {
    this->basetime = basetime;
}

int64_t PlayClockTime::getClockBasetime() {
    return basetime;
}

int64_t PlayClockTime::getClockTime() {
    struct timeval currentTime;
    gettimeofday(&currentTime, 0);

    return currentTime.tv_sec * 1000000 + currentTime.tv_usec;
}

