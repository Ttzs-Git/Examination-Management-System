// data.c
#include <stdio.h>
#include "data.h"
#include <string.h>
// 定义全局变量
Question questionBank[MAX_QUESTIONS];
int qCount = 0;
Student studentList[MAX_STUDENTS];
int sCount = 0;
int examQuestionNum = 5;

void loadFiles() {
    FILE *fp;
    char line[1024]; // 定义一个足够大的缓冲区来存每一行文本

    // 1. 读取试题库
    fp = fopen(FILE_QUESTIONS, "r");
    if (fp) {
        qCount = 0;
        while (qCount < MAX_QUESTIONS && fgets(line, sizeof(line), fp)) {
            // 去掉行末的换行符
            size_t len = strlen(line);
            if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
            
            // 跳过空行
            if (strlen(line) < 5) continue;

            // 使用 sscanf 从字符串中解析数据
            // 注意：这里使用临时变量接收解析结果，成功后再赋值给数组
            Question q;
            int matches = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", 
               q.content, q.optionA, q.optionB, q.optionC, q.optionD, q.answer);

            if (matches == 6) {
                // 解析成功，存入题库
                q.id = qCount + 1;
                q.scoreVal = 10;
                questionBank[qCount] = q;
                qCount++;
            } else {
                // 解析失败（比如格式不对），仅打印警告，但不停止循环！
                // printf("警告: 跳过格式错误的行 (第%d行)\n", qCount + 1); 
            }
        }
        fclose(fp);
    } else {
        fp = fopen(FILE_QUESTIONS, "w");
        if(fp) fclose(fp);
    }

    // 2. 读取学生信息 (保持不变，或者也用fgets优化)
    fp = fopen(FILE_STUDENTS, "r");
    if (fp) {
        sCount = 0;
        while (sCount < MAX_STUDENTS && 
               fscanf(fp, "%s %s %d %d", 
               studentList[sCount].id, 
               studentList[sCount].name, 
               &studentList[sCount].hasTaken, 
               &studentList[sCount].score) == 4) {
            sCount++;
        }
        fclose(fp);
    } else {
        fp = fopen(FILE_STUDENTS, "w");
        if(fp) fclose(fp);
    }
}

void saveQuestions() {
    FILE *fp = fopen(FILE_QUESTIONS, "w");
    if (!fp) return;
    for (int i = 0; i < qCount; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s\n", 
            questionBank[i].content,
            questionBank[i].optionA,
            questionBank[i].optionB,
            questionBank[i].optionC,
            questionBank[i].optionD,
            questionBank[i].answer);
    }
    fclose(fp);
}

void saveStudents() {
    FILE *fp = fopen(FILE_STUDENTS, "w");
    if (!fp) return;
    for (int i = 0; i < sCount; i++) {
        fprintf(fp, "%s %s %d %d\n", 
            studentList[i].id, 
            studentList[i].name, 
            studentList[i].hasTaken, 
            studentList[i].score);
    }
    fclose(fp);
}