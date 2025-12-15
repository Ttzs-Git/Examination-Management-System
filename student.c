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
    char inputID[20];
    printf(COLOR_CYAN "\n请输入学号登录: " COLOR_RESET);
    scanf("%s", inputID);
    while(getchar() != '\n'); 

    int idx = -1;
    for (int i = 0; i < sCount; i++) {
        if (strcmp(studentList[i].id, inputID) == 0) {
            idx = i; break;
        }
    }

    if (idx == -1) {
        printf(COLOR_RED "错误：学号不存在！\n" COLOR_RESET);
        pauseSystem(); return;
    }

    // 登录成功欢迎页
    clearScreen();
    printf(COLOR_GREEN "欢迎回来，%s ！\n" COLOR_RESET, studentList[idx].name);
    
    // 如果已考过，给出提示，但允许进入菜单选择本地练习
    if (studentList[idx].hasTaken) {
        printf(COLOR_YELLOW "【提示】你已完成正式考试，成绩: %d 分。\n" COLOR_RESET, studentList[idx].score);
        printf(COLOR_DIM "(注：你不能再次参加网络考试，但可以使用本地模拟功能)\n\n" COLOR_RESET);
    }

    printf("请选择考试模式:\n");
    printf(COLOR_YELLOW "1." COLOR_RESET " 本地模拟考试 (仅自我检测，不计入成绩)\n");
    
    // 动态显示菜单项状态
    if (studentList[idx].hasTaken) {
        printf(COLOR_DIM    "2. 参加网络考试 (已完成，禁止进入)\n" COLOR_RESET);
    } else {
        printf(COLOR_GREEN  "2." COLOR_RESET " 参加网络考试 (正式考试，计入排名)\n");
    }
    
    printf(COLOR_DIM "------------------------\n" COLOR_RESET);
    printf("0. 返回\n");
    printf("请选择: ");

    int mode;
    if (scanf("%d", &mode) != 1) {
        while(getchar() != '\n');
        return;
    }
    while(getchar() != '\n');

    // ==========================================
    //  【核心修复逻辑】
    // ==========================================
    if (mode == 2) {
        // 严格拦截：如果已考过，绝对不允许进入网络模式
        if (studentList[idx].hasTaken) {
            printf(COLOR_RED "\n🚫 操作失败：你已经拥有正式成绩 (%d分)，不允许重复参加网络考试！\n" COLOR_RESET, studentList[idx].score);
            printf("请联系管理员或选择本地模拟模式。\n");
            pauseSystem();
            return;
        }
        
        // 只有未考过的才能进
        joinNetworkExam(&studentList[idx]);
        return;
    } 
    else if (mode == 0) {
        return;
    }
    // ==========================================

    // --- 本地模拟考试逻辑 (mode == 1) ---
    // (这部分逻辑保持不变，允许已考过的学生进来练习)
    
    printf(COLOR_CYAN "\n>>> 正在启动模拟练习模式 (结果不保存) <<<\n" COLOR_RESET);
    pauseSystem();

    if (qCount < examQuestionNum) {
        printf(COLOR_RED "题库题目不足！\n" COLOR_RESET);
        pauseSystem(); return;
    }

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
        printf(COLOR_BLUE "=== [模拟练习] 第 %d / %d 题 ===\n" COLOR_RESET, i + 1, examQuestionNum);
        printf(COLOR_BOLD "\n%s\n\n" COLOR_RESET, q->content);
        printf("A. %s\nB. %s\nC. %s\nD. %s\n", q->optionA, q->optionB, q->optionC, q->optionD);
        printf(COLOR_YELLOW "\n请输入答案: " COLOR_RESET);
        
        char userAns[10];
        // 模拟考试也可以用 fgets 优化，不过 scanf 这里也能凑合用
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