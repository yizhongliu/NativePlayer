//
// Created by llm on 19-9-5.
//

#ifndef NATIVEPLAYER_PLAYCLOCKTIME_H
#define NATIVEPLAYER_PLAYCLOCKTIME_H

#include <stdint.h>

class PlayClockTime {
public:
    PlayClockTime();
    ~PlayClockTime();

    void setClockBasetime(int64_t basetime);
    int64_t getClockTime();
    int64_t getClockBasetime();

private:
    uint64_t basetime;
};

#endif //NATIVEPLAYER_PLAYCLOCKTIME_H
