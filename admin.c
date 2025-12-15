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


void adminMenu() {
    int choice;
    while (1) {
        clearScreen();
        printf(">> 管理员菜单\n");
        printf("1. 添加试题\n");
        printf("2. 添加考生\n");
        printf("3. 查看所有成绩排名\n");
        printf("4. 搜索考生成绩/排名 (按学号或姓名)\n"); // 新增入口
        printf("5. 修改/重置考生信息\n");
        printf("6. 删除考生\n");
        printf("7. 设置单次考试题数 (当前: %d 题 / 题库总数: %d 题)\n", examQuestionNum, qCount);
        printf("0. 返回主菜单\n");
        printf("------------------------\n");
        printf("请选择: ");
        
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
                break;
            case 0: return;
            default: break;
        }
    }
}

// 查找学生索引的辅助函数
static int findStudentIndex(const char* id) {
    for (int i = 0; i < sCount; i++) {
        if (strcmp(studentList[i].id, id) == 0) {
            return i;
        }
    }
    return -1;
}

// 修改/重置考生 
static void modifyStudent() {
    char id[20];
    printf("\n请输入要修改的考生学号: ");
    scanf("%s", id);

    int idx = findStudentIndex(id);
    if (idx == -1) {
        printf("未找到该考生！\n");
        pauseSystem();
        return;
    }

    printf("\n当前信息 -> 姓名: %s, 状态: %s, 成绩: %d\n", 
           studentList[idx].name, 
           studentList[idx].hasTaken ? "已考" : "未考", 
           studentList[idx].score);

    printf("请选择操作:\n");
    printf("1. 修改姓名\n");
    printf("2. 重置考试状态 (允许重考)\n");
    printf("0. 取消\n");
    printf("选择: ");
    
    int op;
    scanf("%d", &op);

    switch (op) {
        case 1:
            printf("请输入新姓名: ");
            scanf("%s", studentList[idx].name);
            printf("姓名修改成功！\n");
            break;
        case 2:
            studentList[idx].hasTaken = 0;
            studentList[idx].score = 0;
            printf("考生状态已重置，可以重新参加考试！\n");
            break;
        case 0:
            return;
        default:
            printf("无效操作。\n");
            break;
    }
    saveStudents(); // 保存更改到文件
    pauseSystem();
}

// 删除考生 
static void deleteStudent() {
    char id[20];
    printf("\n请输入要删除的考生学号: ");
    scanf("%s", id);

    int idx = findStudentIndex(id);
    if (idx == -1) {
        printf("未找到该考生！\n");
        pauseSystem();
        return;
    }

    printf("确定要删除考生 [%s] 吗？(y/n): ", studentList[idx].name);
    char confirm[10];
    scanf("%s", confirm);

    if (confirm[0] == 'y' || confirm[0] == 'Y') {
        // 删除逻辑：将后面的元素前移
        for (int i = idx; i < sCount - 1; i++) {
            studentList[i] = studentList[i + 1];
        }
        sCount--; // 总数减1
        saveStudents(); // 保存更改
        printf("删除成功！\n");
    } else {
        printf("已取消。\n");
    }
    pauseSystem();
}


static void addQuestion() {
    if (qCount >= MAX_QUESTIONS) {
        printf("题库已满！\n");
        return;
    }
    Question q;
    getchar(); 
    printf("输入题干: "); fgets(q.content, 256, stdin); trimNewline(q.content);
    // 简单检查是否有违规字符
    if (strchr(q.content, '|')) {
        printf("错误：题干中不能包含 '|' 符号！\n");
        pauseSystem();
        return;
    }

    printf("输入选项A: "); fgets(q.optionA, 100, stdin); trimNewline(q.optionA);
    printf("输入选项B: "); fgets(q.optionB, 100, stdin); trimNewline(q.optionB);
    printf("输入选项C: "); fgets(q.optionC, 100, stdin); trimNewline(q.optionC);
    printf("输入选项D: "); fgets(q.optionD, 100, stdin); trimNewline(q.optionD);
    printf("输入标准答案: "); scanf("%s", q.answer);
    for(int i=0; q.answer[i]; i++) q.answer[i] = toupper(q.answer[i]);

    questionBank[qCount++] = q;
    saveQuestions();
    printf("试题添加成功！\n");
    pauseSystem();
}

static void addStudent() {
    if (sCount >= MAX_STUDENTS) {
        printf("学生名额已满！\n");
        return;
    }
    Student s;
    printf("输入学号: "); scanf("%s", s.id);
    // 检查学号重复
    if (findStudentIndex(s.id) != -1) {
        printf("错误：学号已存在！\n");
        pauseSystem();
        return;
    }
    printf("输入姓名: "); scanf("%s", s.name);
    s.hasTaken = 0; s.score = 0;
    studentList[sCount++] = s;
    saveStudents();
    printf("学生添加成功！\n");
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


void listStudents() {
    sortStudentsByScore();
    printf("\n%-15s %-15s %-10s %-10s\n", "学号", "姓名", "状态", "成绩");
    printf("--------------------------------------------------\n");
    for (int i = 0; i < sCount; i++) {
        printf("%-15s %-15s %-10s %-10d\n", 
            studentList[i].id, studentList[i].name, 
            studentList[i].hasTaken ? "已考" : "未考", studentList[i].score);
    }
}

void searchStudent() {
    char input[50];
    printf("\n请输入要查找的 考生学号 或 姓名: ");
    scanf("%s", input);

    // 必须先排序，这样计算出的下标才是排名
    sortStudentsByScore();

    int found = 0;
    printf("\n搜索结果:\n");
    printf("%-5s %-15s %-15s %-10s %-10s\n", "排名", "学号", "姓名", "状态", "成绩");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < sCount; i++) {
        // 同时支持匹配 学号 或 姓名
        if (strcmp(studentList[i].id, input) == 0 || strcmp(studentList[i].name, input) == 0) {
            printf("%-5d %-15s %-15s %-10s %-10d\n", 
                i + 1, // 排名即为排序后的下标+1
                studentList[i].id, 
                studentList[i].name, 
                studentList[i].hasTaken ? "已考" : "未考",
                studentList[i].score);
            found = 1;
        }
    }

    if (!found) {
        printf("未找到匹配的考生信息。\n");
    }
    pauseSystem();
}

