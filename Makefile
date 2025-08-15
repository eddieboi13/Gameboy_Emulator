# === Compiler and Flags ===
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -Iinclude -O2 -DNDEBUG
LDFLAGS := `sdl2-config --libs`

# === Source Files ===
SRC := src/main.c \
       src/cpu.c \
       src/instructions.c \
       src/cb.c \
       src/memory.c \
	   src/timer.c \
	   src/audio.c \
	   src/ppu.c \
	   src/joypad.c 

OBJ := $(SRC:.c=.o)

# === Output ===
TARGET := gameboy

# === Rules ===
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean run

