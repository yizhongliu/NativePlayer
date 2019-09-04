//
// Created by llm on 19-9-3.
//

#include <cstring>
#include "NetTimeProvider.h"
#include <pthread.h>

#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/select.h>
#include<unistd.h>

#include <android/log.h>

//定义日志打印宏函数
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "NetTimeProvider",__VA_ARGS__)


void NetTimeProvider::your_jni_function() {  //这个是你的测试jni函数
    int Recv_Sockfd;
    struct sockaddr_in ser_addr;
    struct sockaddr_in recv_addr;

    socklen_t  addrlen = sizeof(recv_addr);
    int len_read_net;
    unsigned char buffer[1024];
    int res;

    Recv_Sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(Recv_Sockfd <0 ){
        LOGE(" open  socket fail!!!");
        return ;
    }
    ser_addr.sin_family=AF_INET;
    ser_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    ser_addr.sin_port=htons(8090);
    res = bind(Recv_Sockfd,(struct sockaddr*)&ser_addr,sizeof(struct sockaddr_in));
    LOGE("bind res[%u] , sin_port[%u]", res, ser_addr.sin_port);

    len_read_net = recvfrom(Recv_Sockfd, buffer, 1024, 0, (struct sockaddr*)&recv_addr, &addrlen);
//可以再这里加打印看看接收的数据
    LOGE("receive socket %d", len_read_net);

}



NetTimeProvider::NetTimeProvider(char *ip, int port) {
    this->ip = new char[strlen(ip) + 1];
    strcpy(this->ip, ip);

    this->port = port;
}

NetTimeProvider::~NetTimeProvider() {
    if (ip) {
        delete ip;
        ip = 0;
    }
}

void *task_time_provider(void * args) {
    NetTimeProvider *netTimeProvider = static_cast<NetTimeProvider *>(args);

    netTimeProvider->_start();

    return 0;
}

void NetTimeProvider::start() {
    bTimeProviderWrok = true;
    //解码
    pthread_create(&pid_provider, 0, task_time_provider, this);
}

void NetTimeProvider::_start() {
 //   your_jni_function();

    NetTimePacket netTimePacket;
    int len = 0;
    struct timeval currentTime;

    char buff[10];

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == -1) {
        LOGE("create socket fail!!");
        return;
    }

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t  addr_len = sizeof(server_addr);

    LOGE("_start %s, %d", ip, port);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 ) {
        LOGE("bind addr fail!!");
        return;
    }

    while (bTimeProviderWrok) {
        LOGE("before receive");
        if (len = recvfrom(sockfd, &netTimePacket, sizeof(netTimePacket), 0, (struct sockaddr*)&client_addr, &addr_len) < 0) {
            LOGE("recvfrom error");
            continue;
        }

        LOGE("recvfrom %s, ", inet_ntoa(client_addr.sin_addr));
        LOGE("receive local time: %llu", netTimePacket.local_time);

        gettimeofday(&currentTime, 0);

        netTimePacket.remote_time = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
        LOGE("get current time: %d, %d", currentTime.tv_sec, currentTime.tv_usec);

        sendto(sockfd, &netTimePacket, sizeof(netTimePacket), 0, (struct sockaddr*) &client_addr, addr_len);

        LOGE("send remote time: %llu", netTimePacket.remote_time);

    }
    LOGE("leave _start");

    bTimeProviderWrok = false;
    close(sockfd);
}

void NetTimeProvider::stop() {
    bTimeProviderWrok = false;
}

