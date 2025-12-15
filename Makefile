CC = gcc
CFLAGS = -Wall -g
OBJS = main.o data.o utils.o admin.o student.o network.o

TARGET = exam_system

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# 编译规则
main.o: main.c data.h utils.h admin.h student.h network.h
	$(CC) $(CFLAGS) -c main.c

data.o: data.c data.h types.h
	$(CC) $(CFLAGS) -c data.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

admin.o: admin.c admin.h data.h utils.h
	$(CC) $(CFLAGS) -c admin.c

student.o: student.c student.h data.h utils.h
	$(CC) $(CFLAGS) -c student.c

network.o: network.c network.h data.h utils.h
	$(CC) $(CFLAGS) -c network.c

clean:
	rm -f $(OBJS) $(TARGET)