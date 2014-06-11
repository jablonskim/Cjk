#include "utils.h"

void t_check_error(int errnum)
{
    char buf[256];

    if(errnum == 0)
        return;

    strerror_r(errnum, buf, 255);
    fprintf(stderr, "Threads: %s\n", buf);
    exit(EXIT_FAILURE);
}

void t_start_reading(pthread_mutex_t *pmutex, int *prt)
{
    t_check_error(pthread_mutex_lock(pmutex));
    (*prt)++;
    t_check_error(pthread_mutex_unlock(pmutex));
}

void t_stop_reading(pthread_mutex_t *pmutex, pthread_cond_t *pcond, int *prt)
{
    t_check_error(pthread_mutex_lock(pmutex));
    (*prt)--;
    t_check_error(pthread_cond_broadcast(pcond));
    t_check_error(pthread_mutex_unlock(pmutex));
}

void t_start_writing(pthread_mutex_t *pmutex, pthread_cond_t *pcond, int *prt)
{
    t_check_error(pthread_mutex_lock(pmutex));
    while(*prt != 0)
        t_check_error(pthread_cond_wait(pcond, pmutex));
}

void t_sigmask(int signum, int how)
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, signum);
    t_check_error(pthread_sigmask(how, &set, NULL));
}

void t_stop_writing(pthread_mutex_t *pmutex)
{
    t_check_error(pthread_mutex_unlock(pmutex));
}

void start_detached_thread(pthread_mutex_t *mutex, pthread_attr_t *t_attr, 
        pthread_t *thread, void *(*func)(void*), void *params)
{
    t_check_error(pthread_mutex_init(mutex, NULL));
    t_check_error(pthread_mutex_lock(mutex));
    t_check_error(pthread_attr_init(t_attr));
    t_check_error(pthread_attr_setdetachstate(t_attr, PTHREAD_CREATE_DETACHED));
    t_check_error(pthread_create(thread, t_attr, func, params));
    t_check_error(pthread_attr_destroy(t_attr));
    t_check_error(pthread_mutex_unlock(mutex));  
}

void stop_detached_thread(pthread_mutex_t *mutex, void *d)
{
    t_check_error(pthread_mutex_lock(mutex));
    t_check_error(pthread_mutex_unlock(mutex));
    t_check_error(pthread_mutex_destroy(mutex));

    free(d);
}

void error_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void safe_sleep(int seconds, int *pwork)
{
    int tt;
    for(tt = seconds; (!pwork || *pwork) && tt > 0; tt = sleep(tt));
}

ssize_t socket_write(int fd, void *_buf, size_t count)
{
    int c;
    size_t len = 0;
    char *buf = (char*)_buf;

    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));

        if(c < 0)
            return c;

        buf += c;
        len += c;
        count -= c;
    } while(count > 0);

    return len;
}

ssize_t socket_read(int fd, void *_buf, size_t count)
{
    int c;
    size_t len = 0;
    char *buf = (char*)_buf;

    do
    {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));

        if(c < 0)
            return c;

        if(c == 0)
            return len;

        buf += c;
        len += c;
        count -= c;
    } while(count > 0);

    return len;
}

void socket_close(int fd)
{
    if(TEMP_FAILURE_RETRY(close(fd)) < 0)
        error_exit("Closing socket:");
}

int socket_connect_safe(int fd, char *hostname, uint16_t port)
{
    struct sockaddr_in saddr;
    struct hostent *hostinfo, hostinfo_t;
    char buf[512];
    int err;

    if(gethostbyname_r(hostname, &hostinfo_t, buf, 511, &hostinfo, &err) || hostinfo == NULL)
        return 1;

    memset(&saddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr = *(struct in_addr*)hostinfo->h_addr;

    if(TEMP_FAILURE_RETRY(connect(fd, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in))) < 0)
        return 1;

    return 0;
}

void socket_connect(int fd, char *hostname, uint16_t port)
{
    if(socket_connect_safe(fd, hostname, port))
        error_exit("Connecting to server:");
}

int sethandler(void (*f)(int), int sig)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if(sigaction(sig, &act, NULL) == -1)
        return -1;
    return 0;
}

int make_socket(int type)
{
    int sockfd;
    
    if(type != SOCK_STREAM && type != SOCK_DGRAM)
        error_exit("Creating socket: Invalid socket type.");

    if((sockfd = socket(PF_INET, type, 0)) < 0)
        error_exit("Creating socket:");

    return sockfd;
}

int make_and_bind_socket(uint16_t port, int type)
{
    int sockfd;
    struct sockaddr_in addr;

    sockfd = make_socket(type);

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        error_exit("Binding socket:");

    return sockfd;
}

void socket_listen(int sockfd, int max_connections)
{
    if(listen(sockfd, max_connections) < 0)
        error_exit("Listen:");
}
