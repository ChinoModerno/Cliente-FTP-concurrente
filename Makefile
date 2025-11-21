# Makefile para ReyesL-clienteFTP
# Autor: ReyesL

CC = gcc
CFLAGS = -Wall -g
TARGET = ReyesL-clienteFTP
SRC = ReyesL-clienteFTP.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
