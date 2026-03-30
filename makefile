CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRCS = main.c lex.c symtab.c parser.c opt.c diag.c
OBJS = $(SRCS:.c=.o)
TARGET = salc

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.tk *.ts *.trc *.o