SRCS := $(wildcard *.c)
SRCS_CRYPT := $(wildcard ./cypt_lib/*.c)
CC := gcc
COM_P := -Wall -g -pthread 

netdisk_server: $(SRCS) $(SRCS_CRYPT)
	$(CC) $^ -o $@ $(COM_P) 

clean:
	$(RM) netdisk_server

rebuild: clean netdisk_server