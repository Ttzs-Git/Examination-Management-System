#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "data.h"

// ==========================================
// 核心修复：安全的发送函数
// ==========================================
void send_safe(int sock, const char* msg) {
    // 动态分配内存，防止缓冲区溢出
    int len = strlen(msg);
    // 多申请 10 个字节放结束符 "$$$" 和 \0
    char *packet = malloc(len + 10); 
    if (!packet) return;

    // 拼接结束符
    sprintf(packet, "%s$$$", msg);
    
    // 发送全部数据
    write(sock, packet, strlen(packet));
    
    free(packet);
    //稍微睡一点点时间，防止极快速度下的TCP粘包（可选）
    usleep(1000); 
}

// AI 分析脚本调用
void call_ai_analysis(const char* log_file, char* output_buffer) {
    char command[512];
    // 强制指定 UTF-8 环境
    const char* python_path="/home/ttzs/miniconda3/envs/ros2/bin/python";
    sprintf(command, "export PYTHONIOENCODING=utf-8; %s analyze_exam.py %s", python_path, log_file);
    FILE *pipe = popen(command, "r");
    if (pipe) {
        // 读取大量数据
        int n = fread(output_buffer, 1, 10240, pipe); // 增加缓冲区
        if (n > 0) {
            output_buffer[n] = '\0';
        } else {
            strcpy(output_buffer, "AI 脚本运行后没有输出内容。");
        }
        pclose(pipe);
    } else {
        strcpy(output_buffer, "AI Service Error.");
    }
}

void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);
    
    // 接收缓冲区
    char buffer[4096] = {0};
    
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        // 读取客户端请求
        int valread = read(sock, buffer, sizeof(buffer)-1);
        if (valread <= 0) break;
        buffer[valread] = '\0'; // 确保 C 字符串结束

        // 1. 管理员获取学生列表
        if (strncmp(buffer, "ADMIN_GET_STU", 13) == 0) {
            pthread_mutex_lock(&data_lock);
            // 动态分配大内存以防学生太多爆栈
            char *response = malloc(1024 * 50); 
            strcpy(response, "STU_LIST|");
            
            for(int i=0; i<sCount; i++) {
                char temp[200];
                sprintf(temp, "%s,%s,%d,%d;", studentList[i].id, studentList[i].name, studentList[i].hasTaken, studentList[i].score);
                strcat(response, temp);
            }
            pthread_mutex_unlock(&data_lock);
            
            send_safe(sock, response);
            free(response);
        }
        
        // 2. 管理员添加学生
        else if (strncmp(buffer, "ADMIN_ADD_STU|", 14) == 0) {
            char id[20], name[50];
            sscanf(buffer + 14, "%[^|]|%s", id, name);
            
            pthread_mutex_lock(&data_lock);
            int exist = 0;
            for(int i=0; i<sCount; i++) if(strcmp(studentList[i].id, id)==0) exist=1;
            
            if(!exist && sCount < MAX_STUDENTS) {
                strcpy(studentList[sCount].id, id);
                strcpy(studentList[sCount].name, name);
                studentList[sCount].hasTaken = 0;
                studentList[sCount].score = 0;
                sCount++;
                saveStudents();
                send_safe(sock, "OK");
            } else {
                send_safe(sock, "FAIL");
            }
            pthread_mutex_unlock(&data_lock);
        }

        // 3. 管理员重置
        else if (strncmp(buffer, "ADMIN_RESET_STU|", 16) == 0) {
            char id[20];
            sscanf(buffer + 16, "%s", id);
            pthread_mutex_lock(&data_lock);
            for(int i=0; i<sCount; i++) {
                if(strcmp(studentList[i].id, id)==0) {
                    studentList[i].hasTaken = 0;
                    studentList[i].score = 0;
                    saveStudents();
                    break;
                }
            }
            pthread_mutex_unlock(&data_lock);
            send_safe(sock, "OK");
        }

        // 4. 学生登录
        else if (strncmp(buffer, "LOGIN|", 6) == 0) {
            char id[30];
            sscanf(buffer + 6, "%s", id);
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
                
                // === 考试逻辑 ===
                int *indices = malloc(qCount * sizeof(int));
                for(int i=0; i<qCount; i++) indices[i] = i;
                for (int i = qCount - 1; i > 0; i--) { 
                    int j = rand() % (i + 1);
                    int temp = indices[i]; indices[i] = indices[j]; indices[j] = temp;
                }

                int score = 0;
                FILE *log_fp;
                char log_file[50];
                sprintf(log_file, "log_%s.json", id);
                log_fp = fopen(log_file, "w");
                fprintf(log_fp, "[\n");

                for(int i=0; i<examQuestionNum && i<qCount; i++) {
                    int qIdx = indices[i];
                    char q_buf[4096]; 
                    Question *q = &questionBank[qIdx];
                    
                    sprintf(q_buf, "QUE|%s|%s|%s|%s|%s", q->content, q->optionA, q->optionB, q->optionC, q->optionD);
                    send_safe(sock, q_buf);

                    char ans_buf[100] = {0};
                    int r = read(sock, ans_buf, sizeof(ans_buf)-1);
                    if(r <= 0) break;
                    ans_buf[r] = '\0'; // 确保结束

                    fprintf(log_fp, "  {\"question\": \"%s\", \"user_answer\": \"%s\", \"correct_answer\": \"%s\"}%s\n",
                         q->content, ans_buf, q->answer, (i == examQuestionNum - 1) ? "" : ",");
                    
                    if(strcmp(ans_buf, q->answer) == 0) score += 10;
                }
                fprintf(log_fp, "]\n");
                fclose(log_fp);
                free(indices);

                pthread_mutex_lock(&data_lock);
                studentList[idx].score = score;
                studentList[idx].hasTaken = 1;
                saveStudents();
                pthread_mutex_unlock(&data_lock);

                char msg[200];
                sprintf(msg, "MSG|得分: %d 分. 正在生成AI报告...", score);
                send_safe(sock, msg);

                // AI 分析 (加大缓冲区)
                char *report = malloc(1024 * 20); 
                memset(report, 0, 1024 * 20);
                call_ai_analysis(log_file, report);
                
                char *final_pkt = malloc(1024 * 25);
                sprintf(final_pkt, "REPORT|%s", report);
                send_safe(sock, final_pkt);
                
                free(report);
                free(final_pkt);
                break; 
            }
        }
    }
    close(sock);
    return 0;
}