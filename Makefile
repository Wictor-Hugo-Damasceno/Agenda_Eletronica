CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iraylib/include
LDFLAGS = -Lraylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static-libgcc

SRC = Trabalho.c
TARGET = agenda

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

clean:
	del $(TARGET).exe

run: $(TARGET)
	./$(TARGET).exe