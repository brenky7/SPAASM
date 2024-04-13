# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -pedantic

# Source files
CLIENT_SRCS = my_libs/client_lib.c
SERVER_SRCS = my_libs/server_lib.c
UTILS_SRCS = my_libs/utils.c
SHELL_SRC = shell.c

# Object files
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
UTILS_OBJS = $(UTILS_SRCS:.c=.o)
SHELL_OBJ = $(SHELL_SRC:.c=.o)

# Executable
EXECUTABLE = shell

all: $(EXECUTABLE)

$(EXECUTABLE): $(CLIENT_OBJS) $(SERVER_OBJS) $(UTILS_OBJS) $(SHELL_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
# Compile client files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(UTILS_OBJS) $(SHELL_OBJ) $(EXECUTABLE)
