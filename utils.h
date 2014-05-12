#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void error_exit(const char *msg);
int sethandler(void (*f)(int), int sig);
int make_socket(int type);
int make_and_bind_socket(uint16_t port, int type);
void socket_listen(int sockfd, int max_connections);
void safe_sleep(int seconds);
ssize_t socket_write(int fd, void *buf, size_t count);
ssize_t socket_read(int fd, void *_buf, size_t count);
void socket_connect(int fd, char *hostname, uint16_t port);
void socket_close(int fd);

