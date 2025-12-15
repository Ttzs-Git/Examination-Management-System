// network.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>
#include "network.h"
#include "data.h"
#include "utils.h"

#define MAX_BUFFER 2048

// --- 教师端 (Server) 实现 ---
void startNetworkServer() {
    if (qCount < examQuestionNum) {
        printf(COLOR_RED "错误：题库题目不足\n" COLOR_RESET);
        pauseSystem(); return;
    }

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[MAX_BUFFER] = {0};
    char recv_buf[100] = {0};

    // 1. 创建 Socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed"); pauseSystem(); return;
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt"); pauseSystem(); return;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(NET_PORT);

    // 2. 绑定 & 监听
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed"); 
        printf(COLOR_YELLOW "提示：端口占用，请稍后再试或杀掉旧进程。\n" COLOR_RESET);
        pauseSystem(); return;
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen"); pauseSystem(); return;
    }

    clearScreen();
    printf(COLOR_BOLD COLOR_MAGENTA ">>> 网络考试发布端 (教师) <<<\n" COLOR_RESET);
    printf("监听中 (Port %d)... 等待考生...\n", NET_PORT);

    // 3. 接受连接
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept"); return;
    }

    // 4. 读取考生身份 (格式: "ID|NAME")
    memset(recv_buf, 0, sizeof(recv_buf));
    read(new_socket, recv_buf, sizeof(recv_buf));
    
    // --- 解析学号 (用于后续存档) ---
    char currentStudentID[30] = {0};
    // 这里简单处理：复制 | 之前的内容作为学号
    for(int k=0; k<sizeof(recv_buf); k++) {
        if(recv_buf[k] == '|') break;
        currentStudentID[k] = recv_buf[k];
    }
    // ----------------------------

    printf(COLOR_GREEN ">>> 考生已连接: [%s]\n" COLOR_RESET, recv_buf);

    // 5. 准备题目
    int *indices = (int*)malloc(qCount * sizeof(int));
    for(int i=0; i<qCount; i++) indices[i] = i;
    srand(time(NULL));
    for (int i = qCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i]; indices[i] = indices[j]; indices[j] = temp;
    }

    int score = 0;

    // 发送欢迎
    sprintf(buffer, "MSG|" COLOR_BOLD "网络考试已连接。本场考试计入总成绩。\n" COLOR_RESET);
    send(new_socket, buffer, strlen(buffer), 0);
    usleep(100000);

    // 6. 循环发题
    for (int i = 0; i < examQuestionNum; i++) {
        int qIdx = indices[i];
        Question *q = &questionBank[qIdx];

        memset(buffer, 0, MAX_BUFFER);
        sprintf(buffer, "QUE|" 
                COLOR_BLUE "------------------------------\n" COLOR_RESET
                "第 %d/%d 题\n"
                COLOR_BOLD "%s" COLOR_RESET "\n"
                "A. %s\nB. %s\nC. %s\nD. %s\n"
                COLOR_YELLOW "答案: " COLOR_RESET, 
                i + 1, examQuestionNum, 
                q->content, q->optionA, q->optionB, q->optionC, q->optionD);
        
        send(new_socket, buffer, strlen(buffer), 0);

        memset(recv_buf, 0, 100);
        int valread = read(new_socket, recv_buf, 100);
        if(valread <= 0) break;

        trimNewline(recv_buf);
        char ansChar = toupper(recv_buf[0]);
        char correctChar = toupper(q->answer[0]);
        
        printf("考生第%d题回答: %c ", i+1, ansChar);
        if (ansChar == correctChar) {
            score += 10;
            printf(COLOR_GREEN "√\n" COLOR_RESET);
        } else {
            printf(COLOR_RED "X\n" COLOR_RESET);
        }
    }
    free(indices);

    // 7. 发送最终成绩给学生
    memset(buffer, 0, MAX_BUFFER);
    sprintf(buffer, "FIN|" COLOR_GREEN "\n考试结束！你的正式成绩: %d 已存档。\n" COLOR_RESET, score);
    send(new_socket, buffer, strlen(buffer), 0);

    // ==================================================
    // 【核心新增】: 教师端负责将成绩写入数据库 (文件)
    // ==================================================
    int found = 0;
    for(int i=0; i<sCount; i++) {
        if(strcmp(studentList[i].id, currentStudentID) == 0) {
            // 更新内存数据
            studentList[i].score = score;
            studentList[i].hasTaken = 1;
            found = 1;
            break;
        }
    }
    
    if(found) {
        saveStudents(); // 写入 students.txt
        printf(COLOR_GREEN ">>> 成绩已成功录入系统数据库。\n" COLOR_RESET);
    } else {
        printf(COLOR_RED ">>> 警告：未找到该考生学号，成绩未保存！\n" COLOR_RESET);
    }
    // ==================================================

    printf(">>> 连接关闭。\n");
    close(new_socket);
    close(server_fd);
    pauseSystem();
}

// --- 学生端 (Client) 实现 ---
void joinNetworkExam(Student *s) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[MAX_BUFFER] = {0};
    char input_ans[100]; // 缓冲区加大

    // 1. 创建Socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf(COLOR_RED "Socket 创建失败\n" COLOR_RESET); return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(NET_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf(COLOR_RED "无效 IP\n" COLOR_RESET); return;
    }

    clearScreen();
    printf("正在连接教师端 (%s:%d)...\n", SERVER_IP, NET_PORT);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf(COLOR_RED "连接失败！请确认教师已在管理员后台发布考试。\n" COLOR_RESET);
        pauseSystem(); return;
    }

    // --- 新增：连接成功后立即发送身份信息 ---
    char identity[100];
    sprintf(identity, "%s|%s", s->id, s->name);
    send(sock, identity, strlen(identity), 0);
    // ------------------------------------

    printf(COLOR_GREEN "连接成功！等待题目...\n" COLOR_RESET);

    // 4. 通信循环
    while (1) {
        memset(buffer, 0, MAX_BUFFER);
        int valread = read(sock, buffer, MAX_BUFFER - 1);
        if (valread <= 0) {
            printf(COLOR_RED "\n与服务器断开连接。\n" COLOR_RESET);
            break;
        }

        // 协议解析
        if (strncmp(buffer, "FIN|", 4) == 0) {
            printf("%s", buffer + 4);
            break;
        } 
        else if (strncmp(buffer, "QUE|", 4) == 0) {
            printf("%s", buffer + 4); 
            fflush(stdout); // 强制刷新，防止题目不显示

            // --- 核心修复：使用 fgets 替代 scanf ---
            // 清空 input_ans
            memset(input_ans, 0, sizeof(input_ans));
            
            // 读取整行输入 (解决 scanf 遇到空格或回车卡顿的问题)
            // 循环读取直到读到非空内容 (防止上一次的回车残留)
            while(1) {
                if(fgets(input_ans, sizeof(input_ans), stdin) != NULL) {
                     trimNewline(input_ans);
                     if(strlen(input_ans) > 0) break; // 读到了有效内容
                }
            }

            // 发送答案
            send(sock, input_ans, strlen(input_ans), 0);
            printf("已提交，请等待下一题...\n");
        } 
        else if (strncmp(buffer, "MSG|", 4) == 0) {
            printf("%s", buffer + 4);
        }
    }

    close(sock);
    pauseSystem();
}