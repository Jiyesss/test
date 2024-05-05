# Compiler and Compiler Flags
CC=gcc
CFLAGS=-Wno-deprecated-declarations -g -Iutils -Icore
# Linker flags
LDFLAGS=-lreadline

# The build target executable:
TARGET=os

# Source, Object files
SRCS=os.c
OBJS=$(SRCS:.c=.o) 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# To obtain object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up:
clean:
	rm -f $(OBJS) $(TARGET)
