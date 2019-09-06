//
// Created by llm on 19-9-3.
//

#ifndef NATIVEPLAYER_NETTIMEPROVIDER_H
#define NATIVEPLAYER_NETTIMEPROVIDER_H

#include "NetTimePacket.h"



class NetTimeProvider {
public:
    NetTimeProvider(char * ip, int port);
    ~NetTimeProvider();

    void start();
    void _start();
    void stop();

private:
    char * ip;
    int port;
    pthread_t pid_provider;
    bool bTimeProviderWrok;
};

#endif //NATIVEPLAYER_NETTIMEPROVIDER_H
