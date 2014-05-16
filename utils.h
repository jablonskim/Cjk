#ifndef _UTILS_H
#define _Utils_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void error_exit(const char *msg);
void t_check_error(int errnum);
int sethandler(void (*f)(int), int sig);
int make_socket(int type);
int make_and_bind_socket(uint16_t port, int type);
void socket_listen(int sockfd, int max_connections);
void safe_sleep(int seconds, int *pwork);
ssize_t socket_write(int fd, void *buf, size_t count);
ssize_t socket_read(int fd, void *_buf, size_t count);
void socket_connect(int fd, char *hostname, uint16_t port);
void socket_close(int fd);
void start_detached_thread(pthread_mutex_t *mutex, pthread_attr_t *t_attr, pthread_t *thread, void *(*func)(void*), void *params);
void stop_detached_thread(pthread_mutex_t *mutex, void *d);
void t_start_reading(pthread_mutex_t *pmutex, int *prt);
void t_stop_reading(pthread_mutex_t *pmutex, pthread_cond_t *pcond, int *prt);
void t_start_writing(pthread_mutex_t *pmutex, pthread_cond_t *pcond, int *prt);
void t_stop_writing(pthread_mutex_t *pmutex);
void t_sigmask(int signum, int how);

#endif
