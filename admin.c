// admin.c
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "admin.h"
#include "data.h"
#include "utils.h"

// 内部函数声明
static void addQuestion();
static void addStudent();
static void sortStudentsByScore();
static void deleteStudent();
static void modifyStudent();

// 1. 美化菜单
void adminMenu() {
    int choice;
    while (1) {
        clearScreen();
        printf(COLOR_BOLD COLOR_MAGENTA ">> 管理员控制台\n" COLOR_RESET);
        printf(COLOR_DIM "------------------------\n" COLOR_RESET);
        printf(COLOR_YELLOW "1." COLOR_RESET " 添加试题\n");
        printf(COLOR_YELLOW "2." COLOR_RESET " 添加考生\n");
        printf(COLOR_YELLOW "3." COLOR_RESET " 查看所有成绩排名\n");
        printf(COLOR_YELLOW "4." COLOR_RESET " 搜索考生成绩/排名\n");
        printf(COLOR_YELLOW "5." COLOR_RESET " 修改/重置考生信息\n");
        printf(COLOR_RED    "6." COLOR_RESET " 删除考生\n");
        printf(COLOR_CYAN   "7." COLOR_RESET " 设置单次考试题数 (当前: " COLOR_BOLD "%d" COLOR_RESET " / 总题库: %d)\n", examQuestionNum, qCount);
        printf(COLOR_DIM "------------------------\n" COLOR_RESET);
        printf("0. 返回主菜单\n");
        printf(COLOR_DIM "------------------------\n" COLOR_RESET);
        printf("请选择: ");
        
        // 修复之前的 Warning：将 continue 写在花括号里
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); 
            continue;
        }

        switch (choice) {
            case 1: addQuestion(); break;
            case 2: addStudent(); break;
            case 3: listStudents(); pauseSystem(); break;
            case 4: searchStudent(); break;
            case 5: modifyStudent(); break;
            case 6: deleteStudent(); break;
            case 7: 
                printf("输入新的考试题目数量: ");
                scanf("%d", &examQuestionNum);
                if(examQuestionNum > qCount) examQuestionNum = qCount;
                printf(COLOR_GREEN "设置已更新！\n" COLOR_RESET);
                pauseSystem();
                break;
            case 0: return;
            default: break;
        }
    }
}

static int findStudentIndex(const char* id) {
    for (int i = 0; i < sCount; i++) {
        if (strcmp(studentList[i].id, id) == 0) return i;
    }
    return -1;
}

static void modifyStudent() {
    char id[20];
    printf("\n请输入要修改的考生学号: ");
    scanf("%s", id);
    int idx = findStudentIndex(id);
    
    if (idx == -1) {
        printf(COLOR_RED "未找到该考生！\n" COLOR_RESET);
        pauseSystem(); return;
    }

    printf(COLOR_CYAN "\n当前信息 -> 姓名: %s, 状态: %s, 成绩: %d\n" COLOR_RESET, 
           studentList[idx].name, 
           studentList[idx].hasTaken ? "已考" : "未考", 
           studentList[idx].score);

    printf("1. 修改姓名\n");
    printf("2. 重置考试状态 (允许重考)\n");
    printf("0. 取消\n");
    printf("选择: ");
    int op; scanf("%d", &op);

    switch (op) {
        case 1:
            printf("请输入新姓名: "); scanf("%s", studentList[idx].name);
            printf(COLOR_GREEN "姓名修改成功！\n" COLOR_RESET);
            break;
        case 2:
            studentList[idx].hasTaken = 0; studentList[idx].score = 0;
            printf(COLOR_GREEN "考生状态已重置，可以重新参加考试！\n" COLOR_RESET);
            break;
        default: break;
    }
    saveStudents();
    pauseSystem();
}

static void deleteStudent() {
    char id[20];
    printf("\n请输入要删除的考生学号: "); scanf("%s", id);
    int idx = findStudentIndex(id);
    if (idx == -1) {
        printf(COLOR_RED "未找到该考生！\n" COLOR_RESET);
        pauseSystem(); return;
    }

    printf(COLOR_RED "确定要删除考生 [%s] 吗？(y/n): " COLOR_RESET, studentList[idx].name);
    char confirm[10]; scanf("%s", confirm);

    if (confirm[0] == 'y' || confirm[0] == 'Y') {
        for (int i = idx; i < sCount - 1; i++) studentList[i] = studentList[i + 1];
        sCount--;
        saveStudents();
        printf(COLOR_GREEN "删除成功！\n" COLOR_RESET);
    } else {
        printf("已取消。\n");
    }
    pauseSystem();
}

static void addQuestion() {
    if (qCount >= MAX_QUESTIONS) {
        printf(COLOR_RED "题库已满！\n" COLOR_RESET); return;
    }
    Question q;
    getchar(); 
    printf("输入题干: "); fgets(q.content, 256, stdin); trimNewline(q.content);
    if (strchr(q.content, '|')) {
        printf(COLOR_RED "错误：题干中不能包含 '|' 符号！\n" COLOR_RESET);
        pauseSystem(); return;
    }
    printf("输入选项A: "); fgets(q.optionA, 100, stdin); trimNewline(q.optionA);
    printf("输入选项B: "); fgets(q.optionB, 100, stdin); trimNewline(q.optionB);
    printf("输入选项C: "); fgets(q.optionC, 100, stdin); trimNewline(q.optionC);
    printf("输入选项D: "); fgets(q.optionD, 100, stdin); trimNewline(q.optionD);
    printf("输入标准答案: "); scanf("%s", q.answer);
    for(int i=0; q.answer[i]; i++) q.answer[i] = toupper(q.answer[i]);

    questionBank[qCount++] = q;
    saveQuestions();
    printf(COLOR_GREEN "试题添加成功！\n" COLOR_RESET);
    pauseSystem();
}

static void addStudent() {
    if (sCount >= MAX_STUDENTS) {
        printf(COLOR_RED "学生名额已满！\n" COLOR_RESET); return;
    }
    Student s;
    printf("输入学号: "); scanf("%s", s.id);
    if (findStudentIndex(s.id) != -1) {
        printf(COLOR_RED "错误：学号已存在！\n" COLOR_RESET);
        pauseSystem(); return;
    }
    printf("输入姓名: "); scanf("%s", s.name);
    s.hasTaken = 0; s.score = 0;
    studentList[sCount++] = s;
    saveStudents();
    printf(COLOR_GREEN "学生添加成功！\n" COLOR_RESET);
    pauseSystem();
}

static void sortStudentsByScore() {
    for (int i = 0; i < sCount - 1; i++) {
        for (int j = 0; j < sCount - 1 - i; j++) {
            if (studentList[j].score < studentList[j+1].score) {
                Student temp = studentList[j];
                studentList[j] = studentList[j+1];
                studentList[j+1] = temp;
            }
        }
    }
}

// 2. 美化列表输出
void searchStudent() {
    char input[50];
    printf("\n请输入要查找的 考生学号 或 姓名: ");
    scanf("%s", input);
    sortStudentsByScore();

    int found = 0;
    printf(COLOR_BOLD "\n%-5s %-15s %-15s %-10s %-10s\n" COLOR_RESET, "排名", "学号", "姓名", "状态", "成绩");
    printf(COLOR_DIM "----------------------------------------------------------\n" COLOR_RESET);

    for (int i = 0; i < sCount; i++) {
        if (strcmp(studentList[i].id, input) == 0 || strcmp(studentList[i].name, input) == 0) {
            printf("%-5d %-15s %-15s %-10s ", i + 1, studentList[i].id, studentList[i].name, 
                   studentList[i].hasTaken ? "已考" : "未考");
            
            if(studentList[i].score >= 60)
                printf(COLOR_GREEN "%-10d\n" COLOR_RESET, studentList[i].score);
            else
                printf(COLOR_RED "%-10d\n" COLOR_RESET, studentList[i].score);
            found = 1;
        }
    }
    if (!found) printf(COLOR_YELLOW "未找到匹配的考生信息。\n" COLOR_RESET);
    pauseSystem();
}

void listStudents() {
    sortStudentsByScore();
    printf(COLOR_BOLD "\n%-15s %-15s %-10s %-10s\n" COLOR_RESET, "学号", "姓名", "状态", "成绩");
    printf(COLOR_DIM "--------------------------------------------------\n" COLOR_RESET);
    for (int i = 0; i < sCount; i++) {
        printf("%-15s %-15s %-10s ", studentList[i].id, studentList[i].name, 
               studentList[i].hasTaken ? "已考" : "未考");
        
        if(studentList[i].score >= 60)
            printf(COLOR_GREEN "%-10d\n" COLOR_RESET, studentList[i].score);
        else
            printf(COLOR_RED "%-10d\n" COLOR_RESET, studentList[i].score);
    }
}