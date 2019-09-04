//
// Created by llm on 19-9-3.
//

#ifndef NATIVEPLAYER_NETTIMECLIENT_H
#define NATIVEPLAYER_NETTIMECLIENT_H

#include "NetTimePacket.h"
#include <pthread.h>


class NetTimeClient {
public:
    NetTimeClient(char * ip, int port);
    ~NetTimeClient();

    void start();
    void _start();
    void stop();

private:
    char * server_ip;
    int server_port;
    pthread_t pid_client;
    bool bTimeClientWrok;
};

#endif //NATIVEPLAYER_NETTIMECLIENT_H
