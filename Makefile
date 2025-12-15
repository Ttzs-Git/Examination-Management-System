# Makefile
CC = gcc
CFLAGS = -Wall -g

# 目标文件
TARGET = exam_system

# 对象文件列表
OBJS = main.o data.o utils.o admin.o student.o

# 默认生成目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# 单独编译每个模块
main.o: main.c data.h utils.h admin.h student.h
	$(CC) $(CFLAGS) -c main.c

data.o: data.c data.h types.h
	$(CC) $(CFLAGS) -c data.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

admin.o: admin.c admin.h data.h utils.h
	$(CC) $(CFLAGS) -c admin.c

student.o: student.c student.h data.h utils.h
	$(CC) $(CFLAGS) -c student.c

# 清理
clean:
	rm -f $(OBJS) $(TARGET)