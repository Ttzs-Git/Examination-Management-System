#ifndef NETWORK_H
#define NETWORK_H

#include "types.h" // 需要用到 Student 结构体

#define NET_PORT 8888
#define SERVER_IP "127.0.0.1"
#define NET_BUFFER_NORMAL 1024 // 用于常规消息、题目、答案
#define NET_BUFFER_LARGE  4096 // 专门用于容纳AI报告
// 启动教师端服务 (Server)
void startNetworkServer();

// 启动学生端连接 (Client) - 传入当前登录的学生信;
void joinNetworkExam(const char* studentID);
void serveExamToStudent(int sock, Student* s);
void examLoop(int sock);
#endif
