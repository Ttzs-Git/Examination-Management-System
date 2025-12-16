// utils.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h> 
#include <sys/ioctl.h>
#include "utils.h"

void clearScreen() {
    system("clear"); // 清空系统终端屏幕 
}

void pauseSystem() {
    // 处理换行残留
    printf("\n按回车键继续...");
    getchar();// 等待用户按回车键继续
    getchar(); // 处理换行符带来的影响
}

void trimNewline(char *str) {
    // 移除字符串末尾的换行符
    size_t len = strlen(str);
    if (len > 0 && str[len-1] == '\n') {// 总是带'\n'需要手动截断
        str[len-1] = '\0';
    }
}

// fgets() 读取一行，包括换行符，但不包含 '\0'
// scanf() 读取一行，不包括换行符，但包含 '\0'

void getPassword(char *password, int maxLength) {
    // 使用termios库
    struct termios oldt, newt;
    int i = 0;
    int c;

    // 1. 获取当前终端配置
    tcgetattr(STDIN_FILENO, &oldt);// 保存原来的配置
    newt = oldt;

    // 2. 关闭 回显(ECHO) 和 规范模式(ICANON)
    // ECHO关:不显示输入
    // ICANON关:逐字符读，不等待回车
    newt.c_lflag &= ~(ECHO | ICANON);// 关回显+关闭规范模式

    
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

int kbhit() {
    // 检查是否有键入等待读取, 非阻塞检测按键
    static const int STDIN = 0;
    static int initialized = 0;
    int bytesWaiting;

    if (!initialized) {
        // 使用 termios 来配置终端，使其变为非规范模式
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = 1;
    }

    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}