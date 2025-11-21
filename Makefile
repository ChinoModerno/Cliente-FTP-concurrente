# Makefile ReyesL-clienteFTP

CC = gcc
CFLAGS = -Wall -g
TARGET = ReyesL-clienteFTP

# Lista de todos los archivos fuente necesarios
SRCS = ReyesL-clienteFTP.c connectsock.c connectTCP.c passivesock.c passiveTCP.c errexit.c

# Generación automática de la lista de objetos (.o)
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(TARGET) $(OBJS)