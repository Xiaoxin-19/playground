SRCS := $(wildcard *.c)
CC := gcc
COM_P := -Wall -g -pthread -I ./header

server: $(SRCS)
	$(CC) $^ -o $@ $(COM_P) 

clean:
	$(RM) server

rebuild: clean all
