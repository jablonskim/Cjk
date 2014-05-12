#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

void error_exit(const char *msg);
int sethandler(void (*f)(int), int sig);
int make_socket(int type);
int make_and_bind_socket(uint16_t port, int type);
void socket_listen(int sockfd, int max_connections);

