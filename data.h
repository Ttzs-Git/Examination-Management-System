// data.h
#ifndef DATA_H
#define DATA_H

#include "types.h"

// 声明全局变量 (extern)
extern Question questionBank[MAX_QUESTIONS];
extern int qCount;
extern Student studentList[MAX_STUDENTS];
extern int sCount;
extern int examQuestionNum;

// 函数声明
void loadFiles();
void saveStudents();
void saveQuestions();

#endif