//data.c
#include <stdio.h>
#include <string.h>
#include "data.h"
// 数据模型与持久化层: 数据结构体定义和数据的读取与写入

// 定义全局变量
Question questionBank[MAX_QUESTIONS];
int qCount = 0;
Student studentList[MAX_STUDENTS];
int sCount = 0;

int examQuestionNum = 5;// 默认考试题目数量
int g_exam_started = 0; // 全局考试状态标志

// 字符串清洗
void trim(char* str) {
    // fgets 去除末尾换行符
    size_t len = strlen(str);
    if(len > 0 && str[len-1] == '\n') str[len-1] = '\0';
}

// 加载数据
void loadFiles() {
    // 从硬盘中读取题目并且放到内存中
    FILE *fp = fopen(FILE_QUESTIONS, "r");
    char line[1024];
    qCount = 0;
    if (fp) {
        while (qCount < MAX_QUESTIONS && fgets(line, sizeof(line), fp)) {
            trim(line);//去掉换行符
            if(strlen(line) < 5) continue;
            Question q;
            int fields = sscanf(line, "%255[^|]|%99[^|]|%99[^|]|%99[^|]|%99[^|]|%9s", 
               q.content, q.optionA, q.optionB, q.optionC, q.optionD, q.answer);
               // 使用高级格式化读取，防止溢出
               //scanf系列函数的正则化表达式
               // | 按分隔符读取多个字段
               // %(格式说明开始)255(最多读取255个字符)[^|(读到|为止)]
               // 预留1字节给\0结束符
               // | 普通字符(字符串中匹配|字符)
               // %9s 读取字符串直到空白符
        
            if (fields == 6) {
                // 要求全部字段读取成功
                q.id = qCount + 1;
                questionBank[qCount++] = q;
            }
        }
        fclose(fp);
    }
    // 从硬盘中读取学生数据并且放到内存中
    fp = fopen(FILE_STUDENTS, "r");
    sCount = 0;
    if (fp) {
        while (sCount < MAX_STUDENTS && fscanf(fp, "%19s %49s %d %d", 
               studentList[sCount].id, studentList[sCount].name, 
               &studentList[sCount].hasTaken, &studentList[sCount].score) == 4) {
            sCount++;
        } //不支持带空格的名字
        fclose(fp);
    }
}


// 持久化
void saveQuestions() {
    FILE *fp = fopen(FILE_QUESTIONS, "w");
    if (!fp) return;
    for (int i = 0; i < qCount; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s\n", questionBank[i].content, questionBank[i].optionA,
            questionBank[i].optionB, questionBank[i].optionC, questionBank[i].optionD, questionBank[i].answer);
    }
   
    fclose(fp);
}

void saveStudents() {
    FILE *fp = fopen(FILE_STUDENTS, "w");
    if (!fp) return;
    for (int i = 0; i < sCount; i++) {
        fprintf(fp, "%s %s %d %d\n", studentList[i].id, studentList[i].name, 
            studentList[i].hasTaken, studentList[i].score);
    }
    fclose(fp);
}