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

void error_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void safe_sleep(int seconds)
{
    int tt;
    for(tt = seconds; tt > 0; tt = sleep(tt));
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

void socket_connect(int fd, char *hostname, uint16_t port)
{
    struct sockaddr_in saddr;
    struct hostent *hostinfo;

    if((hostinfo = gethostbyname(hostname)) == NULL)
    {
        fprintf(stderr, "Gethostbyname error.\n");
        exit(EXIT_FAILURE);
    }

    memset(&saddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr = *(struct in_addr*)hostinfo->h_addr;

    if(connect(fd, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in)) < 0)
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
