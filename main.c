// main.c
#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#include "utils.h"
#include "admin.h"
#include "student.h"

// 美化后的公共查询菜单
void publicQueryMenu() {
    int choice;
    while(1) {
        clearScreen();
        printf(COLOR_BLUE "======================================\n" COLOR_RESET);
        printf(COLOR_BOLD COLOR_CYAN "       成绩查询中心 (公共入口)         \n" COLOR_RESET);
        printf(COLOR_BLUE "======================================\n" COLOR_RESET);
        printf(COLOR_YELLOW "1." COLOR_RESET " 输入 学号/姓名 查询个人成绩\n");
        printf(COLOR_YELLOW "2." COLOR_RESET " 查看全校及格/排名榜\n");
        printf(COLOR_DIM "--------------------------------------\n" COLOR_RESET);
        printf(COLOR_RED "0. 返回主菜单\n" COLOR_RESET);
        printf(COLOR_DIM "--------------------------------------\n" COLOR_RESET);
        printf("请选择: ");
        
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch(choice) {
            case 1: searchStudent(); break;
            case 2: listStudents(); pauseSystem(); break;
            case 0: return;
            default: break;
        }
    }
}

int main() {
    loadFiles(); // 启动时读取数据

    while (1) {
        clearScreen();
        // 使用 Bold Cyan (粗体青色) 作为主标题
        printf(COLOR_BLUE "======================================\n" COLOR_RESET);
        printf(COLOR_BOLD COLOR_CYAN "    考试管理系统 " COLOR_MAGENTA "(Linux)" COLOR_RESET "\n");
        printf(COLOR_BLUE "======================================\n" COLOR_RESET);
        
        printf(COLOR_BOLD "主要功能:\n" COLOR_RESET);
        printf("  " COLOR_YELLOW "1." COLOR_RESET " 管理员入口 " COLOR_DIM "(教师/教务)\n" COLOR_RESET);
        printf("  " COLOR_YELLOW "2." COLOR_RESET " 考生入口   " COLOR_DIM "(参加考试)\n" COLOR_RESET);
        printf("  " COLOR_YELLOW "3." COLOR_RESET " 成绩查询   " COLOR_DIM "(所有人可用)\n" COLOR_RESET);
        
        printf(COLOR_DIM "--------------------------------------\n" COLOR_RESET);
        printf("  " COLOR_RED "0. 退出系统\n" COLOR_RESET);
        printf(COLOR_DIM "--------------------------------------\n" COLOR_RESET);
        printf(COLOR_BOLD "请输入选项编号: " COLOR_RESET);

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
                printf(COLOR_GREEN "\n退出系统成功，数据已保存。再见！\n" COLOR_RESET);
                exit(0);
            default: 
                printf(COLOR_RED "无效输入，请重试！\n" COLOR_RESET); 
                pauseSystem();
        }
    }
    return 0;
}