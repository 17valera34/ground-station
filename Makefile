CC=gcc
CFLAGS=-Wall -O2 -I./inc
# Добавляем библиотеку ncurses
LDFLAGS=-lncurses -lpthread -lm
SRC=$(shell find . -name "*.c")
TARGET=app

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)