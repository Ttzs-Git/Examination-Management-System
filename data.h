// data.h

#ifndef DATA_H
#define DATA_H

#include <pthread.h>

#define MAX_QUESTIONS 1000
#define MAX_STUDENTS 500
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
    char id[20];
    char name[50];
    int hasTaken;
    int score;
} Student;

extern Question questionBank[MAX_QUESTIONS];
extern int qCount;
extern Student studentList[MAX_STUDENTS];
extern int sCount;
extern int examQuestionNum;
extern pthread_mutex_t data_lock; // 全局锁

void loadFiles();
void saveQuestions();
void saveStudents();

#endif