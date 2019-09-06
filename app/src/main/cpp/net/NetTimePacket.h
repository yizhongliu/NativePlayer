//
// Created by llm on 19-9-3.
//

#ifndef NATIVEPLAYER_NETTIMEPACKET_H
#define NATIVEPLAYER_NETTIMEPACKET_H

#include<sys/types.h>

typedef struct _NetTimePacket NetTimePacket;

/**
 * GstNetTimePacket:
 * @local_time: the local time when this packet was sent
 * @remote_time: the remote time observation
 *
 * Content of a #GstNetTimePacket.
 */
struct _NetTimePacket {
    int64_t  local_time;
    int64_t  remote_time;
};


#endif //NATIVEPLAYER_NETTIMEPACKET_H
