#ifndef NETWORK_H
#define NETWORK_H

#include "types.h" // 需要用到 Student 结构体

#define NET_PORT 8888
#define SERVER_IP "127.0.0.1"

// 启动教师端服务 (Server)
void startNetworkServer();

// 启动学生端连接 (Client) - 传入当前登录的学生信息
void joinNetworkExam(Student *s);

#endif
