#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

void error_exit(const char *msg);
int sethandler(void (*f)(int), int sig);

