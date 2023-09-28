# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -O2

# Source files
SRCS = main.c jx9.c unqlite.c

# Object files (generated from source files)
OBJS = $(SRCS:.c=.o)

# Target binary
TARGET = quiz

# Default target (build the binary)
all: $(TARGET)

# Link the object files to create the binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony target to prevent conflicts with files named 'clean' or 'all'
.PHONY: clean all
