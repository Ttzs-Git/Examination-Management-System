// data.h
#ifndef DATA_H
#define DATA_H
// 标准防御机制(防止重复定义)
#include <pthread.h> // 提供了创建、管理、同步线程所需的函数、类型和宏定义 
// 内存模型: 不支持最大扩容--快但是灵活性略低
#define MAX_QUESTIONS 1000 // 最大题目数
#define MAX_STUDENTS 500 // 最大学生数
#define FILE_QUESTIONS "questions.txt"
#define FILE_STUDENTS "students.txt"
typedef struct {
    int id;
    char content[256];
    char optionA[100];
    char optionB[100];
    char optionC[100];
    char optionD[100];
    char answer[10];
} Question;

typedef struct {
    char id[20];// 兼容学号字符串
    char name[50];
    int hasTaken;
    int score;
} Student;

// extern 全局变量的声明
extern Question questionBank[MAX_QUESTIONS];
extern int qCount;
extern Student studentList[MAX_STUDENTS];
extern int sCount;

extern int examQuestionNum;
extern pthread_mutex_t data_lock; // 全局互斥锁: 并发安全
extern int g_exam_started; 

void loadFiles();
void saveQuestions();
void saveStudents();

#endif