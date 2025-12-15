// main.c
#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#include "utils.h"
#include "admin.h"
#include "student.h"
void publicQueryMenu() {
    int choice;
    while(1) {
        clearScreen();
        printf("======================================\n");
        printf("       成绩查询中心 (公共入口)         \n");
        printf("======================================\n");
        printf("1. 输入 学号/姓名 查询个人成绩\n");
        printf("2. 查看全校及格/排名榜\n");
        printf("0. 返回主菜单\n");
        printf("--------------------------------------\n");
        printf("请选择: ");
        
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch(choice) {
            case 1: searchStudent(); break; // 直接调用 admin.c 中暴露出来的函数
            case 2: listStudents(); pauseSystem(); break; // 同上
            case 0: return;
            default: break;
        }
    }
}

int main() {
    loadFiles(); // 启动时读取数据

    while (1) {
        clearScreen();
        printf("======================================\n");
        printf("                考试管理系统            \n");
        printf("======================================\n");
        printf("1. 管理员入口\n");
        printf("2. 考生入口\n");
        printf("3. 成绩查询\n");
        printf("0. 退出系统\n");
        printf("--------------------------------------\n");
        printf("请选择: ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: adminMenu(); break;
            case 2: studentMenu(); break;
            case 3: publicQueryMenu(); break;
            case 0: 
                saveStudents(); 
                printf("退出系统...\n");
                exit(0);
            default: printf("无效输入！\n"); pauseSystem();
        }
    }
    return 0;
}