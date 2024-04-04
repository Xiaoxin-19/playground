#ifndef COMMOM_H
#define COMMOM_H
#include "socket_m.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <syslog.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <ctype.h>
#include <aio.h>
#include <sys/epoll.h>
#include <sys/un.h>

// my hesder file
#include "socket_m.h"
#include "proc_pool.h"
#include "error.h"
#include "signal_m.h"
#include "epoll_m.h"
#include "queue.h"
#define SA struct sockaddr

#endif