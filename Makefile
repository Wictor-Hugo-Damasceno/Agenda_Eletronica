CC = dependencias\compiler\bin\gcc.exe
CFLAGS = -Wall -Wextra -std=c11 -I. -Idependencias/raylib/include
LDFLAGS = -Ldependencias/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static-libgcc

SRC = Trabalho.c
TARGET = agenda.exe

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

clean:
	if exist $(TARGET) del $(TARGET)

run: $(TARGET)
	./$(TARGET)