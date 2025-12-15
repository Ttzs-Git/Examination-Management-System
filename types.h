// types.h
#ifndef TYPES_H
#define TYPES_H

#define MAX_QUESTIONS 1000
#define MAX_STUDENTS 500
#define FILE_QUESTIONS "questions.txt"
#define FILE_STUDENTS "students.txt"

// 试题结构体
typedef struct {
    int id;
    char content[256];
    char optionA[100];
    char optionB[100];
    char optionC[100];
    char optionD[100];
    char answer[10];
    int scoreVal;
} Question;

// 考生结构体
typedef struct {
    char id[20];
    char name[50];
    int hasTaken;
    int score;
} Student;

#endif