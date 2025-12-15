// student.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "student.h"
#include "data.h"
#include "utils.h"

void studentMenu() {
    char inputID[20];
    printf("\n请输入学号登录: ");
    scanf("%s", inputID);

    int idx = -1;
    for (int i = 0; i < sCount; i++) {
        if (strcmp(studentList[i].id, inputID) == 0) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("学号不存在！\n");
        pauseSystem();
        return;
    }

    if (studentList[idx].hasTaken) {
        printf("你已参加过考试，成绩: %d\n", studentList[idx].score);
        pauseSystem();
        return;
    }

    if (qCount < examQuestionNum) {
        printf("题库题目不足！\n");
        pauseSystem();
        return;
    }

    // 考试逻辑：随机抽题
    int *indices = (int*)malloc(qCount * sizeof(int));
    for(int i=0; i<qCount; i++) indices[i] = i;

    srand(time(NULL));
    for (int i = qCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    int currentScore = 0;
    for (int i = 0; i < examQuestionNum; i++) {
        int qIdx = indices[i];
        Question *q = &questionBank[qIdx];
        
        clearScreen();
        printf("[第 %d/%d 题]\n%s\n\n", i + 1, examQuestionNum, q->content);
        printf("A. %s\nB. %s\nC. %s\nD. %s\n", q->optionA, q->optionB, q->optionC, q->optionD);
        printf("\n请输入答案: ");
        
        char userAns[10];
        scanf("%s", userAns);
        for(int k=0; userAns[k]; k++) userAns[k] = toupper(userAns[k]);

        if (strcmp(userAns, q->answer) == 0) currentScore += 10;
    }
    free(indices);

    studentList[idx].score = currentScore;
    studentList[idx].hasTaken = 1;
    saveStudents();
    printf("\n考试结束！成绩: %d\n", currentScore);
    pauseSystem();
}