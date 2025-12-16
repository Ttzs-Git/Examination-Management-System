// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>  // 【新增】引入 time.h
#include "data.h"

#define PORT 8888

// 互斥锁，保护共享数据
pthread_mutex_t data_lock = PTHREAD_MUTEX_INITIALIZER;

void *client_handler(void *socket_desc);

int main() {
    int server_fd, new_socket, *new_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // 【修复】初始化随机数种子，否则每次重启抽题顺序都一样
    srand((unsigned int)time(NULL));

    signal(SIGPIPE, SIG_IGN); // 忽略管道破裂信号

    loadFiles();
    printf(">> [C Backend] 数据加载完成。题目数: %d, 学生数: %d\n", qCount, sCount);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { perror("socket failed"); exit(1); }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { perror("bind failed"); exit(1); }
    if (listen(server_fd, 10) < 0) { perror("listen failed"); exit(1); }

    printf(">> [C Backend] 服务器已启动，监听端口 %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        pthread_t sniffer_thread;
        new_sock = malloc(sizeof(int));
        *new_sock = new_socket;

        if (pthread_create(&sniffer_thread, NULL, client_handler, (void *)new_sock) < 0) {
            perror("could not create thread");
            free(new_sock);
            continue;
        }
        pthread_detach(sniffer_thread);
        printf(">> [Event] 新客户端连接 (Socket: %d)\n", new_socket);
    }
    return 0;
}