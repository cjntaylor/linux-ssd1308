SRCS = ssd1308.c
NAME = ssd1308

#########################################################

CC     = gcc
OBJS   = $(SRCS:.c=.o)

all: static test

static: $(OBJS)
	ar rcs lib$(NAME).a $<
	ranlib lib$(NAME).a

test: static test.o
	$(CC) -o $(NAME)_test -Os test.o lib$(NAME).a

%.o: %.c
	$(CC) -c -o $@ $< -g

clean:
	rm -rf $(OBJS) lib$(NAME).a $(NAME)_test test.o
