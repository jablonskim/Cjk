#include "utils.h"

void error_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

ssize_t socket_write(int fd, char *buf, size_t count)
{
    int c;
    size_t len = 0;

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
