
CC := arm-linux-gnueabihf-gcc
#CC  =gcc

CFLAGS  = -Wall -g 
OBJS  =n720-test.o sysfs_io.o 
TARGET=n720-test

all: $(TARGET)

$(TARGET):$(OBJS) 
	$(CC) -o $@ $^

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm n720-test *.o
