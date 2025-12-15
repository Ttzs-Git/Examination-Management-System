// utils.h
#ifndef UTILS_H
#define UTILS_H

#define COLOR_RESET   "\x1b[0m"
#define COLOR_BOLD    "\x1b[1m"
#define COLOR_DIM     "\x1b[2m"

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"

// --- 函数声明 ---
void clearScreen();
void pauseSystem();
void trimNewline(char *str);

#endif