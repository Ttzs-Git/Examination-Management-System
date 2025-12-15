// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "utils.h"
#include "admin.h"
#include "student.h"
#include "network.h"


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
        while(getchar() != '\n'); 
        switch (choice) {
            case 1: 
            {
                    char inputPwd[30];
                    printf(COLOR_YELLOW "请输入管理员密码: " COLOR_RESET);
                    
                    // 调用刚才写的隐藏输入函数
                    getPassword(inputPwd, 30);

                    // 校验密码
                    if (strcmp(inputPwd, ADMIN_PASSWORD) == 0) {
                        printf(COLOR_GREEN "√ 验证成功！正在进入...\n" COLOR_RESET);
                        // 稍微停顿一下，增加交互感 (使用 sleep 需要 <unistd.h>，这里简单用空循环或不暂停均可)
                        adminMenu(); 
                    } else {
                        printf(COLOR_RED "X 密码错误！拒绝访问。\n" COLOR_RESET);
                        pauseSystem();
                    }
                }
            break;
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