#include "signal_m.h"
#include <signal.h>
#include <stdio.h>

int signal_m(int signum, void (*handler)(int))
{
  struct sigaction act;
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  return sigaction(signum, &act, NULL);
}