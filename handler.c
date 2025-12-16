// handler.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include "data.h"

#define DELIMITER "$$$"

// ==========================================
// 【修复】安全的发送函数 (循环 write)
// ==========================================
void send_safe(int sock, const char* msg) {
    if (!msg) return;
    int len = strlen(msg);
    char *packet = malloc(len + 10); 
    if (!packet) return;

    // 拼接协议结束符
    sprintf(packet, "%s%s", msg, DELIMITER);
    
    size_t total_len = strlen(packet);
    size_t sent = 0;
    
    // 【修复】循环发送，确保大数据包（如学生列表）不会被截断
    while (sent < total_len) {
        ssize_t n = write(sock, packet + sent, total_len - sent);
        if (n <= 0) break;
        sent += n;
    }
    
    free(packet);
}

// ==========================================
// 【修复】安全的接收函数 (解决粘包问题)
// 读取直到遇到 "$$$" 或 缓冲区满
// 返回：有效数据长度，-1 表示错误/断开
// ==========================================
int recv_packet(int sock, char *buf, int max_len) {
    int idx = 0;
    char c;
    int dollar_cnt = 0;
    memset(buf, 0, max_len);

    while (idx < max_len - 1) {
        // 逐字节读取是解决粘包最简单的方案（虽然效率略低，但对于此场景足够）
        int n = read(sock, &c, 1);
        if (n <= 0) return -1; // 连接断开
        
        buf[idx++] = c;
        
        // 检测 $$$
        if (c == '$') dollar_cnt++;
        else dollar_cnt = 0;
        
        if (dollar_cnt == 3) {
            buf[idx - 3] = '\0'; // 截断 $$$，保留有效内容
            return idx - 3;
        }
    }
    return -1; // 包太长或未找到结束符
}

// AI 分析脚本调用 (保持不变)
void call_ai_analysis(const char* log_file, char* output_buffer,int max_size) {
    char command[512];
    const char* python_path="/home/ttzs/miniconda3/envs/ros2/bin/python";
    sprintf(command, "export PYTHONIOENCODING=utf-8; %s analyze_exam.py %s", python_path, log_file);
    FILE *pipe = popen(command, "r");
    if (pipe) {
        int n = fread(output_buffer, 1, max_size-1, pipe);
        if (n > 0) output_buffer[n] = '\0';
        else strcpy(output_buffer, "AI 脚本运行后没有输出内容。");
        pclose(pipe);
    } else {
        strcpy(output_buffer, "AI Service Error.");
    }
}

void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);
    char buffer[4096]; 
    
    while (1) {
        // 【修复】使用 recv_packet 替代 read，解决粘包
        int valread = recv_packet(sock, buffer, sizeof(buffer));
        if (valread <= 0) break; // 断开连接

        // ---------------- 管理员功能 ----------------

        if (strncmp(buffer, "ADMIN_GET_STU", 13) == 0) {
            pthread_mutex_lock(&data_lock);
            char *response = malloc(1024 * 200); // 增加缓冲区大小
            if (response) {
                sprintf(response, "STU_LIST|%d,%d|", qCount, examQuestionNum);
                for(int i=0; i<sCount; i++) {
                    char temp[256];
                    // 防止单个条目过长
                    snprintf(temp, sizeof(temp), "%s,%s,%d,%d;", 
                        studentList[i].id, studentList[i].name, studentList[i].hasTaken, studentList[i].score);
                    strcat(response, temp);
                }
                pthread_mutex_unlock(&data_lock);
                send_safe(sock, response);
                free(response);
            } else {
                pthread_mutex_unlock(&data_lock);
                send_safe(sock, "FAIL|Server Memory Error");
            }
        }
        
        else if (strncmp(buffer, "ADMIN_ADD_STU|", 14) == 0) {
            char id[20], name[50];
            // 【修复】sscanf 增加长度限制，防止溢出
            sscanf(buffer + 14, "%19[^|]|%49s", id, name);
            pthread_mutex_lock(&data_lock);
            int exist = 0;
            for(int i=0; i<sCount; i++) if(strcmp(studentList[i].id, id)==0) exist=1;
            if(!exist && sCount < MAX_STUDENTS) {
                strcpy(studentList[sCount].id, id);
                strcpy(studentList[sCount].name, name);
                studentList[sCount].hasTaken = 0; studentList[sCount].score = 0;
                sCount++;
                saveStudents();
                send_safe(sock, "OK");
            } else send_safe(sock, "FAIL");
            pthread_mutex_unlock(&data_lock);
        }

        else if (strncmp(buffer, "ADMIN_DEL_STU|", 14) == 0) {
            char id[20];
            sscanf(buffer + 14, "%19s", id);
            pthread_mutex_lock(&data_lock);
            int found = -1;
            for(int i=0; i<sCount; i++) {
                if(strcmp(studentList[i].id, id)==0) { found = i; break; }
            }
            if(found != -1) {
                for(int k=found; k<sCount-1; k++) studentList[k] = studentList[k+1];
                sCount--;
                saveStudents();
                send_safe(sock, "OK");
            } else send_safe(sock, "FAIL");
            pthread_mutex_unlock(&data_lock);
        }

        else if (strncmp(buffer, "ADMIN_RESET_STU|", 16) == 0) {
            char id[20];
            sscanf(buffer + 16, "%19s", id);
            pthread_mutex_lock(&data_lock);
            for(int i=0; i<sCount; i++) {
                if(strcmp(studentList[i].id, id)==0) {
                    studentList[i].hasTaken = 0; studentList[i].score = 0;
                    saveStudents(); break;
                }
            }
            pthread_mutex_unlock(&data_lock);
            send_safe(sock, "OK");
        }

        else if (strncmp(buffer, "ADMIN_SET_COUNT|", 16) == 0) {
            int newNum;
            sscanf(buffer + 16, "%d", &newNum);
            pthread_mutex_lock(&data_lock);
            if (newNum > 0 && newNum <= qCount) {
                examQuestionNum = newNum;
                send_safe(sock, "OK");
            } else {
                send_safe(sock, "FAIL|数量无效");
            }
            pthread_mutex_unlock(&data_lock);
        }

        else if (strncmp(buffer, "ADMIN_ADD_QUE|", 14) == 0) {
            if (qCount >= MAX_QUESTIONS) {
                send_safe(sock, "FAIL|题库已满");
            } else {
                Question q;
                // 【修复】增加 sscanf 安全限制
                sscanf(buffer + 14, "%255[^|]|%99[^|]|%99[^|]|%99[^|]|%99[^|]|%9s", 
                    q.content, q.optionA, q.optionB, q.optionC, q.optionD, q.answer);
                
                pthread_mutex_lock(&data_lock);
                q.id = qCount + 1;
                questionBank[qCount++] = q;
                saveQuestions();
                pthread_mutex_unlock(&data_lock);
                send_safe(sock, "OK");
            }
        }

        // ---------------- 考生功能 ----------------
        else if (strncmp(buffer, "LOGIN|", 6) == 0) {
            char id[30];
            sscanf(buffer + 6, "%29s", id);
            int idx = -1;
            
            pthread_mutex_lock(&data_lock);
            for(int i=0; i<sCount; i++) {
                if(strcmp(studentList[i].id, id) == 0) { idx = i; break; }
            }
            pthread_mutex_unlock(&data_lock);

            if (idx == -1) send_safe(sock, "LOGIN_FAIL|学号不存在");
            else if (studentList[idx].hasTaken) send_safe(sock, "LOGIN_FAIL|已考过试");
            else {
                char resp[200];
                sprintf(resp, "LOGIN_OK|%s", studentList[idx].name);
                send_safe(sock, resp);
                
                // 抽题
                int *indices = malloc(qCount * sizeof(int));
                for(int i=0; i<qCount; i++) indices[i] = i;
                // 洗牌算法
                for (int i = qCount - 1; i > 0; i--) { 
                    int j = rand() % (i + 1);
                    int temp = indices[i]; indices[i] = indices[j]; indices[j] = temp;
                }

                int score = 0;
                // 【修复】增加完成计数器
                int completed_count = 0; 

                // 准备日志
                char log_file[50];
                sprintf(log_file, "log_%s.json", id);
                FILE *log_fp = fopen(log_file, "w");
                if (log_fp) fprintf(log_fp, "[\n");

                for(int i=0; i<examQuestionNum && i<qCount; i++) {
                    int qIdx = indices[i];
                    char q_buf[4096]; 
                    Question *q = &questionBank[qIdx];
                    
                    sprintf(q_buf, "QUE|%s|%s|%s|%s|%s", q->content, q->optionA, q->optionB, q->optionC, q->optionD);
                    send_safe(sock, q_buf);

                    // 【修复】接收答案也使用 recv_packet，防止与下一条指令粘包
                    char ans_buf[256];
                    int r = recv_packet(sock, ans_buf, sizeof(ans_buf));
                    
                    if(r <= 0) {
                        printf("Student %s disconnected unexpectedly.\n", id);
                        break; 
                    }

                    if (log_fp) {
                        fprintf(log_fp, "  {\"question\": \"%s\", \"user_answer\": \"%s\", \"correct_answer\": \"%s\"}%s\n",
                            q->content, ans_buf, q->answer, (i == examQuestionNum - 1) ? "" : ",");
                    }
                    
                    if(strcmp(ans_buf, q->answer) == 0) score += 10;
                    completed_count++; // 标记该题完成
                }

                if (log_fp) {
                    fprintf(log_fp, "]\n");
                    fclose(log_fp);
                }
                free(indices);

                // 【修复】只有当实际完成题目数等于考试要求数时，才保存成绩
                // 防止断线后被判为 0 分且无法重考
                if (completed_count == examQuestionNum) {
                    pthread_mutex_lock(&data_lock);
                    studentList[idx].score = score;
                    studentList[idx].hasTaken = 1;
                    saveStudents();
                    pthread_mutex_unlock(&data_lock);

                    char msg[200];
                    sprintf(msg, "MSG|得分: %d 分. 正在生成AI报告...", score);
                    send_safe(sock, msg);

                    char *report = malloc(1024 * 20); 
                    if (report) {
                        memset(report, 0, 1024 * 20);
                        call_ai_analysis(log_file, report,2024 * 20);
                        
                        char *final_pkt = malloc(1024 * 25);
                        if (final_pkt) {
                            sprintf(final_pkt, "REPORT|%s", report);
                            send_safe(sock, final_pkt);
                            free(final_pkt);
                        }
                        free(report);
                    }
                }
                sleep(2); // 等待AI报告生成
                break; // 考试结束，退出循环断开连接
            }
        }
    }
    close(sock);
    return 0;
}