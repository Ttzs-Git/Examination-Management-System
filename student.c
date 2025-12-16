// student.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "student.h"
#include "data.h"
#include "utils.h"
#include "network.h"

void studentMenu() {
    // 显示彩色菜单
    char inputID[20];
    clearScreen();
    printf(COLOR_GREEN "欢迎来到考生入口\n" COLOR_RESET);
    printf("请选择操作:\n");
    printf(COLOR_YELLOW "1." COLOR_RESET " 本地模拟考试 (练习模式)\n");
    printf(COLOR_GREEN  "2." COLOR_RESET " 参加网络考试 (正式考试)\n");
    printf(COLOR_DIM "------------------------\n" COLOR_RESET);
    printf("0. 返回\n");
    printf("请选择: ");

    // 读取用户输入
    int mode;
    if (scanf("%d", &mode) != 1) {
        while(getchar() != '\n'); 
        return;
    }
    while(getchar() != '\n');

    if(mode==1){
    // 本地模拟考试
    printf(COLOR_CYAN "\n>>> 正在启动模拟练习模式 (结果不保存) <<<\n" COLOR_RESET);
    pauseSystem();

    if (qCount < examQuestionNum) {
        printf(COLOR_RED "题库题目不足！\n" COLOR_RESET);
        pauseSystem(); return;
    }

    int *indices = (int*)malloc(qCount * sizeof(int));
    for(int i=0; i<qCount; i++) indices[i] = i;
    srand(time(NULL));
    // fisher-yates算法
    for (int i = qCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i]; indices[i] = indices[j]; indices[j] = temp;
    }
    

    int currentScore = 0;
    for (int i = 0; i < examQuestionNum; i++) {
        int qIdx = indices[i];
        Question *q = &questionBank[qIdx];
        
        clearScreen();
        printf(COLOR_BLUE "=== [模拟练习] 第 %d / %d 题 ===\n" COLOR_RESET, i + 1, examQuestionNum);
        printf(COLOR_BOLD "\n%s\n\n" COLOR_RESET, q->content);
        printf("A. %s\nB. %s\nC. %s\nD. %s\n", q->optionA, q->optionB, q->optionC, q->optionD);
        printf(COLOR_YELLOW "\n请输入答案: " COLOR_RESET);
        
        char userAns[10];
        scanf("%s", userAns);
        for(int k=0; userAns[k]; k++) userAns[k] = toupper(userAns[k]);

        if (strcmp(userAns, q->answer) == 0) currentScore += 10;
    }
    free(indices);

    clearScreen();
    printf(COLOR_BLUE "==================================\n" COLOR_RESET);
    printf(COLOR_BOLD "       模拟练习结束\n" COLOR_RESET);
    printf(COLOR_BLUE "==================================\n" COLOR_RESET);
    printf("你的练习得分是: " COLOR_BOLD COLOR_YELLOW "%d 分\n" COLOR_RESET, currentScore);
    printf(COLOR_DIM "\n(注意：这是模拟成绩，系统未存档)\n" COLOR_RESET);
    
    pauseSystem();
}   
    else if (mode == 2) {
        // 参加网络考试
        printf(COLOR_CYAN "\n请输入你的学号以连接到考试服务器: " COLOR_RESET);
        fgets(inputID, sizeof(inputID), stdin);
        trimNewline(inputID);

        if (strlen(inputID) > 0) {
            // 将学号传给网络模块，让服务器去验证
            joinNetworkExam(inputID);
        return;
    } 
    else{
        // 直接返回
        return;
    }
    
    }
}