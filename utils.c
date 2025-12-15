// utils.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include <termios.h>
#include <unistd.h> 

void clearScreen() {
    system("clear"); 
}

void pauseSystem() {
    printf("\n按回车键继续...");
    getchar();
    getchar(); 
}

void trimNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len-1] == '\n') {
        str[len-1] = '\0';
    }
}

void getPassword(char *password, int maxLength) {
    struct termios oldt, newt;
    int i = 0;
    int c;

    // 1. 获取当前终端配置
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // 2. 关闭 回显(ECHO) 和 规范模式(ICANON)
    // ICANON关闭后，输入字符不需要按回车就能被程序捕获，方便我们手动打印 '*'
    newt.c_lflag &= ~(ECHO | ICANON);
    
    // 3. 应用新配置
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (i < maxLength - 1) {
        c = getchar();

        // 处理回车 (结束输入)
        if (c == '\n' || c == '\r') {
            break;
        }
        // 处理退格键 (Backspace: ASCII 127 或 8)
        else if (c == 127 || c == 8) {
            if (i > 0) {
                printf("\b \b"); // 光标左移，打印空格覆盖星号，再左移
                i--;
            }
        }
        // 处理普通字符
        else {
            password[i++] = c;
            printf("*"); // 手动打印星号
        }
    }
    password[i] = '\0'; // 字符串结束符

    // 4. 恢复终端原有配置
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n"); // 换行
}