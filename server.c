// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // 系统调用接口
#include <pthread.h> // 多线程并发
#include <arpa/inet.h> // 网络字节序
#include <signal.h> // 信号处理
#include <time.h>  
#include "data.h"
#define PORT 8888

// 服务器入口与分发器

// 互斥锁，保护共享数据: 设置锁的默认属性
pthread_mutex_t data_lock = PTHREAD_MUTEX_INITIALIZER;

void *client_handler(void *socket_desc);

int main() {
    // 只负责: 初始化服务器，监听端口，接受连接，创建线程处理客户端请求
    int server_fd, new_socket, *new_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    srand((unsigned int)time(NULL));
    g_exam_started = 0;
    signal(SIGPIPE, SIG_IGN); // 忽略管道破裂信号-->用户端突然断开，防止服务器崩溃

    loadFiles(); // 加载题目和学生数据
    printf(">> [C Backend] 数据加载完成。题目数: %d, 学生数: %d\n", qCount, sCount);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { perror("socket failed"); exit(1); }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // 网络优化
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { perror("bind failed"); exit(1); }
    if (listen(server_fd, 10) < 0) { perror("listen failed"); exit(1); }

    printf(">> [C Backend] 服务器已启动，监听端口 %d...\n", PORT);

    while (1) {
        // 阻塞等待
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        // 堆内存传参
        pthread_t sniffer_thread;
        new_sock = malloc(sizeof(int));
        *new_sock = new_socket;

        // 线程创建
        if (pthread_create(&sniffer_thread, NULL, client_handler, (void *)new_sock) < 0) {
            // 并发模型：每个客户端连接创建一个线程
            perror("could not create thread");
            free(new_sock);
            continue;
        }
        pthread_detach(sniffer_thread);// 线程分离
        printf(">> [Event] 新客户端连接 (Socket: %d)\n", new_socket);
    }
    return 0;
}