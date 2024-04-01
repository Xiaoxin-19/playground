SRCS := $(wildcard *.c)
OUTS := $(patsubst %.c,%,$(SRCS))
CC := gcc
COM_P := -Wall -g
 # Object files
OBJS = $(SRCS:.c=.o) 
all: $(OUTS)
%: %.c
	$(CC) $^ -o $@ $(COM_P) -pthread

clean:
	$(RM) $(OUTS) $(OBJS)

rebuild: clean all
