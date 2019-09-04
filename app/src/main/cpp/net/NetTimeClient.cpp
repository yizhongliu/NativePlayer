//
// Created by llm on 19-9-3.
//
#include "NetTimeClient.h"

#include <cstring>

#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/select.h>
#include<unistd.h>



#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "NetTimeClient",__VA_ARGS__)

NetTimeClient::NetTimeClient(char * ip, int port) {
    this->server_ip = new char[strlen(ip) + 1];
    strcpy(server_ip, ip);

    this->server_port = port;
}

NetTimeClient::~NetTimeClient() {
    if (server_ip) {
        delete server_ip;
        server_ip = 0;
    }
}

void *task_time_client(void * args) {
    NetTimeClient *netTimeProvider = static_cast<NetTimeClient *>(args);

    netTimeProvider->_start();

    return 0;
}

void NetTimeClient::start() {
    bTimeClientWrok = true;
    //解码
    pthread_create(&pid_client, 0, task_time_client, this);
}

void NetTimeClient::_start() {
    LOGE("NetTimeClient::_start() %s, %d", server_ip, server_port);
    int client_sockfd = 0;
    int len = 0;


    struct sockaddr_in server_addr;
    socklen_t  addr_len = sizeof(server_addr);
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    LOGE("sin_port[%u]", server_addr.sin_port);

    client_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_sockfd == -1) {
        LOGE("create socket fail!!");
        return;
    }

    struct timeval tv_out;
    tv_out.tv_sec = 1;//等待10秒
    tv_out.tv_usec = 0;
    setsockopt(client_sockfd, SOL_SOCKET,SO_RCVTIMEO, &tv_out, sizeof(tv_out));

    NetTimePacket netTimePacket;
    struct timeval currentTime;

    while (bTimeClientWrok) {
        gettimeofday(&currentTime, 0);

        netTimePacket.local_time = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
        LOGE("send local time %llu:", netTimePacket.local_time);

        if ((len = sendto(client_sockfd, &netTimePacket, sizeof(netTimePacket), 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr))) < 0 ) {
            LOGE("send to error");
            continue;
        }
        LOGE("send len %d", len);

        len=recvfrom(client_sockfd, &netTimePacket, sizeof(netTimePacket), 0, (struct sockaddr*)&server_addr, &addr_len);
        if (len > 0) {
            LOGE("receive remote_time: %llu", netTimePacket.remote_time);
        }
    }

    bTimeClientWrok = false;
    close(client_sockfd);
}

void NetTimeClient::stop() {
    bTimeClientWrok = false;
}


