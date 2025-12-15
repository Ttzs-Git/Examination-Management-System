// utils.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

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