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
    printf(COLOR_CYAN "\n请输入学号登录考试系统: " COLOR_RESET);
    scanf("%s", inputID);

    int idx = -1;
    for (int i = 0; i < sCount; i++) {
        if (strcmp(studentList[i].id, inputID) == 0) {
            idx = i; break;
        }
    }

    if (idx == -1) {
        printf(COLOR_RED "错误：学号不存在！请联系管理员。\n" COLOR_RESET);
        pauseSystem(); return;
    }

    if (studentList[idx].hasTaken) {
        printf(COLOR_YELLOW "提示：你好 %s，你已完成考试，成绩为: %d 分。\n" COLOR_RESET, 
               studentList[idx].name, studentList[idx].score);
        pauseSystem(); return;
    }

    if (qCount < examQuestionNum) {
        printf(COLOR_RED "系统错误：题库题目不足，无法开启考试！\n" COLOR_RESET);
        pauseSystem(); return;
    }

    printf(COLOR_GREEN "登录成功！考生: %s\n" COLOR_RESET, studentList[idx].name);
    printf("即将开始考试，共 %d 题，请做好准备...\n", examQuestionNum);
    pauseSystem();

    // 随机抽题
    int *indices = (int*)malloc(qCount * sizeof(int));
    for(int i=0; i<qCount; i++) indices[i] = i;

    srand(time(NULL));
    for (int i = qCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i]; indices[i] = indices[j]; indices[j] = temp;
    }

    int currentScore = 0;
    for (int i = 0; i < examQuestionNum; i++) {
        int qIdx = indices[i];
        Question *q = &questionBank[qIdx];
        
        clearScreen();
        // 考试界面美化
        printf(COLOR_BLUE "==================================================\n" COLOR_RESET);
        printf("  " COLOR_BOLD "第 %d / %d 题" COLOR_RESET "  (每题10分)\n", i + 1, examQuestionNum);
        printf(COLOR_BLUE "==================================================\n" COLOR_RESET);
        
        printf(COLOR_BOLD "\n%s\n\n" COLOR_RESET, q->content); // 题干加粗
        
        printf(COLOR_CYAN "  A. " COLOR_RESET "%s\n", q->optionA);
        printf(COLOR_CYAN "  B. " COLOR_RESET "%s\n", q->optionB);
        printf(COLOR_CYAN "  C. " COLOR_RESET "%s\n", q->optionC);
        printf(COLOR_CYAN "  D. " COLOR_RESET "%s\n", q->optionD);
        
        printf(COLOR_YELLOW "\n请输入答案 (如 AB): " COLOR_RESET);
        
        char userAns[10];
        scanf("%s", userAns);
        for(int k=0; userAns[k]; k++) userAns[k] = toupper(userAns[k]);

        // 这里我们不告诉学生对了还是错了，只记录分数
        if (strcmp(userAns, q->answer) == 0) currentScore += 10;
    }
    free(indices);

    studentList[idx].score = currentScore;
    studentList[idx].hasTaken = 1;
    saveStudents();

    clearScreen();
    printf(COLOR_GREEN "\n🎉 考试结束！\n" COLOR_RESET);
    
    // 根据分数给出不同颜色的反馈
    if(currentScore >= 60) {
        printf("你的成绩是: " COLOR_BOLD COLOR_GREEN "%d 分" COLOR_RESET " (通过)\n", currentScore);
    } else {
        printf("你的成绩是: " COLOR_BOLD COLOR_RED "%d 分" COLOR_RESET " (未通过)\n", currentScore);
    }
    
    pauseSystem();
}