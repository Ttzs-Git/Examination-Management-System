// data.c

#include <stdio.h>
#include "data.h"

// 定义全局变量
Question questionBank[MAX_QUESTIONS];
int qCount = 0;
Student studentList[MAX_STUDENTS];
int sCount = 0;
int examQuestionNum = 5;

void loadFiles() {
    FILE *fp;
    
    // 1. 读取试题库
    fp = fopen(FILE_QUESTIONS, "r");
    if (fp) {
        qCount = 0;
        // 核心修改：增加边界检查和返回值检查
        while (qCount < MAX_QUESTIONS && 
               fscanf(fp, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s\n", 
               questionBank[qCount].content,
               questionBank[qCount].optionA,
               questionBank[qCount].optionB,
               questionBank[qCount].optionC,
               questionBank[qCount].optionD,
               questionBank[qCount].answer) == 6) {
            
            questionBank[qCount].id = qCount + 1;
            questionBank[qCount].scoreVal = 10;
            qCount++;
        }
        fclose(fp);
    } else {
        // 如果文件不存在，创建空文件防止报错
        fp = fopen(FILE_QUESTIONS, "w");
        if(fp) fclose(fp);
    }

    // 2. 读取学生信息
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