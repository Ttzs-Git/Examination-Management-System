// network.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>
#include "network.h"
#include "data.h"
#include "utils.h"




// =====================================================================
// 教师端 (Server) 主函数
// =====================================================================
void startNetworkServer() {
     int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[NET_BUFFER_NORMAL] = {0};

    // Socket 创建、绑定、监听...
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { perror("socket failed"); pauseSystem(); return; }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) { perror("setsockopt"); pauseSystem(); return; }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(NET_PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { perror("bind failed"); pauseSystem(); return; }
    if (listen(server_fd, 5) < 0) { perror("listen"); pauseSystem(); return; }

    clearScreen();
    printf(COLOR_BOLD COLOR_MAGENTA ">>> 网络考试发布端 (教师) <<<\n" COLOR_RESET);
    printf("状态: " COLOR_GREEN "服务已启动" COLOR_RESET " | 端口: %d\n", NET_PORT);
    printf("----------------------------------\n");
    printf(COLOR_YELLOW ">>> 服务器正在监听中... 按 'q' 键并回车可随时关闭服务 <<<\n\n" COLOR_RESET);

    fd_set readfds;
    struct timeval tv;
    
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(server_fd + 1, &readfds, NULL, NULL, &tv);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept failed");
            } else {
                memset(buffer, 0, sizeof(buffer));
                read(new_socket, buffer, sizeof(buffer) - 1);

                if (strncmp(buffer, "LOGIN|", 6) == 0) {
                    char studentID[30] = {0};
                    sscanf(buffer + 6, "%s", studentID);
                    int studentIdx = -1;
                    for(int i = 0; i < sCount; i++) {
                        if(strcmp(studentList[i].id, studentID) == 0) {
                            studentIdx = i;
                            break;
                        }
                    }

                    if (studentIdx == -1) {
                        send(new_socket, "LOGIN_FAIL|NOT_FOUND", 20, 0);
                    } else if (studentList[studentIdx].hasTaken) {
                        send(new_socket, "LOGIN_FAIL|TAKEN", 16, 0);
                    } else {
                        char ok_response[100];
                        sprintf(ok_response, "LOGIN_OK|%s", studentList[studentIdx].name);
                        send(new_socket, ok_response, strlen(ok_response), 0);
                        serveExamToStudent(new_socket, &studentList[studentIdx]);
                    }
                }
                close(new_socket);
                printf("\n----------------------------------\n");
                printf("当前考生服务结束。继续监听中 (按 'q' 退出)...\n");
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char c = getchar();
            if (c == 'q' || c == 'Q') {
                // 清理可能遗留的回车符
                while(getchar() != '\n' && getchar() != EOF);
                printf(COLOR_RED "\n>>> 检测到关闭指令，正在关闭服务器...\n" COLOR_RESET);
                break;
            }
        }
    }
    
    close(server_fd);
    printf(">>> 服务器已成功关闭。\n");
    pauseSystem();
}
// =====================================================================
// 为单个学生提供考试服务的核心函数 (Server-side)
// =====================================================================
void serveExamToStudent(int sock, Student* s) {
    printf(COLOR_GREEN ">>> 考生 [%s] 身份验证通过，正在分发试题...\n" COLOR_RESET, s->name);
     char buffer[NET_BUFFER_NORMAL] = {0};       // 用于发送题目和消息
    char report_buffer[NET_BUFFER_LARGE] = {0}; // 用于接收和发送AI报告
    // 1. 准备随机题目
    int *indices = (int*)malloc(qCount * sizeof(int));
    if (indices == NULL) { /* handle malloc error */ return; }
    for(int i=0; i<qCount; i++) indices[i] = i;
    srand(time(NULL));
    for (int i = qCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i]; indices[i] = indices[j]; indices[j] = temp;
    }

    int score = 0;
   

    // 2. 创建JSON日志文件
    FILE *log_fp;
    char log_filename[50];
    sprintf(log_filename, "exam_log_%s.json", s->id);
    log_fp = fopen(log_filename, "w");
    if (log_fp) fprintf(log_fp, "[\n");

    // 3. 循环发题、判分、记录日志
    for (int i = 0; i < examQuestionNum; i++) {
        int qIdx = indices[i];
        Question *q = &questionBank[qIdx];

        sprintf(buffer, "QUE|" 
            COLOR_BLUE "------------------------------\n" COLOR_RESET
            "第 %d/%d 题\n"
            COLOR_BOLD "%s" COLOR_RESET "\n"
            "A. %s\nB. %s\nC. %s\nD. %s\n"
            COLOR_YELLOW "答案: " COLOR_RESET, 
            i + 1, examQuestionNum, 
            q->content, q->optionA, q->optionB, q->optionC, q->optionD);
        send(sock, buffer, strlen(buffer), 0);

        char recv_buf[100] = {0};
        int valread = read(sock, recv_buf, 100);
        if(valread <= 0) { printf(COLOR_RED "考生意外断开连接。\n" COLOR_RESET); break; }

        trimNewline(recv_buf);
        if (strcasecmp(recv_buf, q->answer) == 0) score += 10;
        
        if (log_fp) {
            fprintf(log_fp, "  {\"question\": \"%s\", \"user_answer\": \"%s\", \"correct_answer\": \"%s\"}%s\n",
                    q->content, recv_buf, q->answer, (i == examQuestionNum - 1) ? "" : ",");
        }
    }
    free(indices);
    if (log_fp) { fprintf(log_fp, "]\n"); fclose(log_fp); }

    // 4. 发送分数和等待提示
    sprintf(buffer, "MSG|" COLOR_BOLD "考试结束！得分: %d 分。\n" COLOR_RESET
            COLOR_YELLOW "--- 正在生成AI智能分析报告，请稍候... ---\n" COLOR_RESET, score);
    send(sock, buffer, strlen(buffer), 0);

    // 5. 调用Python脚本并捕获报告
    char command[256];
    sprintf(command,
        "%s analyze_exam.py %s",
        "/home/ttzs/miniconda3/envs/ros2/bin/python",
        log_filename); 
    FILE *pipe = popen(command, "r");
    if (pipe) {
        fread(report_buffer, 1, sizeof(report_buffer) - 1, pipe);
        pclose(pipe);

        // 6. 发送AI报告给学生
        char final_report_packet[NET_BUFFER_LARGE + 10]; // 加上协议头空间
        sprintf(final_report_packet, "REPORT|%s", report_buffer);
        send(sock, final_report_packet, strlen(final_report_packet), 0);
    }

    // 7. 发送最终结束信号
    usleep(100000);
    send(sock, "FIN|", 4, 0);

    // 8. 服务器端更新并保存成绩
    s->score = score;
    s->hasTaken = 1;
    saveStudents();
    printf(COLOR_GREEN ">>> 考生 [%s] 成绩 %d 已成功录入系统。\n" COLOR_RESET, s->name, score);
}


// =====================================================================
// 学生端 (Client) 主函数
// =====================================================================
void joinNetworkExam(const char* studentID) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[NET_BUFFER_NORMAL] = {0};

    // 1. 创建并连接Socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { printf("Socket创建失败\n"); return; }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(NET_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) { printf("无效IP\n"); return; }

    clearScreen();
    printf("正在连接教师端进行身份验证...\n");
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf(COLOR_RED "连接失败！请确认教师端已发布考试。\n" COLOR_RESET);
        pauseSystem(); return;
    }

    // 2. 发送登录请求
    sprintf(buffer, "LOGIN|%s", studentID);
    send(sock, buffer, strlen(buffer), 0);

    // 3. 接收服务器验证结果
    memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, sizeof(buffer) - 1);

    if (strncmp(buffer, "LOGIN_OK|", 9) == 0) {
        char studentName[50];
        sscanf(buffer + 9, "%s", studentName);
        printf(COLOR_GREEN "验证成功！欢迎你，%s 同学。\n" COLOR_RESET, studentName);
        printf("考试即将开始...\n");
        // 4. 进入答题循环
        examLoop(sock);
    } else {
        if (strstr(buffer, "TAKEN")) {
            printf(COLOR_RED "登录失败：你已参加过正式考试，无法再次进入。\n" COLOR_RESET);
        } else if (strstr(buffer, "NOT_FOUND")) {
            printf(COLOR_RED "登录失败：学号不存在，请联系管理员添加。\n" COLOR_RESET);
        } else {
            printf(COLOR_RED "登录失败：未知服务器错误。\n" COLOR_RESET);
        }
        close(sock);
        pauseSystem();
    }
}

// =====================================================================
// 客户端的答题循环函数 (Client-side)
// =====================================================================
void examLoop(int sock) {
    char report_buffer[NET_BUFFER_LARGE] = {0};
    while (1) {
        memset(report_buffer, 0, sizeof(report_buffer));
        int valread = read(sock, report_buffer, sizeof(report_buffer) - 1);
        if (valread <= 0) {
            printf(COLOR_RED "\n与服务器的连接已断开。\n" COLOR_RESET);
            break;
        }

        if (strncmp(report_buffer, "FIN|", 4) == 0) {
            printf(COLOR_DIM "\n(考试流程结束，按回车返回...)\n" COLOR_RESET);
            break;
        } 
        else if (strncmp(report_buffer, "REPORT|", 7) == 0) {
            printf("%s", report_buffer + 7); 
            fflush(stdout);
        }
        else if (strncmp(report_buffer, "QUE|", 4) == 0) {
            printf("%s", report_buffer + 4); 
            fflush(stdout);

            char input_ans[100] = {0};
            while(1) {
                if(fgets(input_ans, sizeof(input_ans), stdin) != NULL) {
                     trimNewline(input_ans);
                     if(strlen(input_ans) > 0) break;
                }
            }
            send(sock, input_ans, strlen(input_ans), 0);
            printf("已提交，请等待...\n");
        } 
        else if (strncmp(report_buffer, "MSG|", 4) == 0) {
            printf("%s", report_buffer + 4);
            fflush(stdout);
        }
    }
    pauseSystem();
}