//data.c
#include <stdio.h>
#include <string.h>
#include "data.h"

Question questionBank[MAX_QUESTIONS];
int qCount = 0;
Student studentList[MAX_STUDENTS];
int sCount = 0;
int examQuestionNum = 5;

void trim(char* str) {
    size_t len = strlen(str);
    if(len > 0 && str[len-1] == '\n') str[len-1] = '\0';
}

void loadFiles() {
    FILE *fp = fopen(FILE_QUESTIONS, "r");
    char line[1024];
    qCount = 0;
    if (fp) {
        while (qCount < MAX_QUESTIONS && fgets(line, sizeof(line), fp)) {
            trim(line);
            if(strlen(line) < 5) continue;
            Question q;
            sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%s", 
               q.content, q.optionA, q.optionB, q.optionC, q.optionD, q.answer);
            q.id = qCount + 1;
            questionBank[qCount++] = q;
        }
        fclose(fp);
    }

    fp = fopen(FILE_STUDENTS, "r");
    sCount = 0;
    if (fp) {
        while (sCount < MAX_STUDENTS && fscanf(fp, "%s %s %d %d", 
               studentList[sCount].id, studentList[sCount].name, 
               &studentList[sCount].hasTaken, &studentList[sCount].score) == 4) {
            sCount++;
        }
        fclose(fp);
    }
}

void saveQuestions() {
    FILE *fp = fopen(FILE_QUESTIONS, "w");
    if (!fp) return;
    for (int i = 0; i < qCount; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s\n", questionBank[i].content, questionBank[i].optionA,
            questionBank[i].optionB, questionBank[i].optionC, questionBank[i].optionD, questionBank[i].answer);
    }
    fclose(fp);
}

void saveStudents() {
    FILE *fp = fopen(FILE_STUDENTS, "w");
    if (!fp) return;
    for (int i = 0; i < sCount; i++) {
        fprintf(fp, "%s %s %d %d\n", studentList[i].id, studentList[i].name, 
            studentList[i].hasTaken, studentList[i].score);
    }
    fclose(fp);
}