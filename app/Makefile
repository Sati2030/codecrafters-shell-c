CC = gcc

CFLAGS = -g -Wall -Werror -MMD

TARGET = debug_prog
SRCS = main.c argumenter.c commands.c input_reader.c prog_interfacing.c redirection.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJS) $(TARGET) $(DEPS)

.PHONY: clean