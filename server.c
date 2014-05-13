#include "utils.h"
#include "vehicle_list.h"

volatile sig_atomic_t work = 1;

void sigint_handler(int sig)
{
    work = 0;
}

void usage(const char *filename)
{
    fprintf(stderr, "USAGE: %s port timestep\n", filename);
    fprintf(stderr, "\tport - port nasluchu.\n");
    fprintf(stderr, "\ttimestep - czas odpytywania [s].\n");

    exit(EXIT_FAILURE);
}

void *background_work(void *data)
{
    return NULL;
}

void *collect_coordinates(void *data)
{
    return NULL;
}

void register_vehicle(int fd, struct vehicle_list *vehicles)
{
    int16_t port;
    char *addr;
    uint32_t data_size, id;

    if(socket_read(fd, &port, sizeof(int16_t)) != sizeof(int16_t))
        return;
    port = ntohs(port);
    if(socket_read(fd, &data_size, sizeof(uint32_t)) != sizeof(uint32_t))
        return;
    data_size = ntohl(data_size);

    addr = (char*)malloc(data_size + 1);
    if(addr == NULL)
        error_exit("Memory allocation:");

    if(socket_read(fd, addr, data_size) != data_size)
    {
        free(addr);
        return;
    }
    addr[data_size] = 0;

    id = htonl(l_add_vehicle(vehicles, port, addr));
    socket_write(fd, "k", 1);
    socket_write(fd, &id, sizeof(uint32_t));

    free(addr);
}

void delete_vehicle(int fd, struct vehicle_list *vehicles)
{
}

void get_history(int fd)
{
}

void calculate_distance(int fd)
{
}

void check_status(int fd)
{
}

void accept_connection(int fd, struct vehicle_list *vehicles)
{
    char command;

    if(socket_read(fd, &command, 1) == 1)
    {
        switch(command)
        {
            case 'r':
                register_vehicle(fd, vehicles);
                break;

            case 'd':
                delete_vehicle(fd, vehicles);
                break;

            case 'h':
                get_history(fd);
                break;

            case 't':
                calculate_distance(fd);
                break;

            case 's':
                check_status(fd);
                break;
        }
    }

    socket_close(fd);
}

void wait_for_connections(uint16_t port, struct vehicle_list *vehicles)
{
    int sockfd, fd;
    pthread_t thread;

    sockfd = make_and_bind_socket(port, SOCK_STREAM);
    socket_listen(sockfd, 16);

    while(work)
    {
        fd = 0;
        while(work && (fd = accept(sockfd, NULL, NULL)) < 0)
        {
            if(errno == EINTR)
                continue;

            error_exit("Accepting connection:");
        }

        if(fd > 0)
            accept_connection(fd, vehicles);
    }

    socket_close(sockfd);
}

void server_work(uint16_t port, int timestep)
{
    struct vehicle_list vehicles;
    init_vehicles_list(&vehicles);

    wait_for_connections(port, &vehicles);

    free_vehicles_list(&vehicles);
}

int main(int argc, char **argv)
{
    int port, timestep;

    if(argc != 3)
        usage(argv[0]);

    port = atoi(argv[1]);
    timestep = atoi(argv[2]);

    if(port <= 0 || timestep <= 0)
        usage(argv[0]);

    if(sethandler(SIG_IGN, SIGPIPE))
        error_exit("Setting SIGPIPE handler:");
    if(sethandler(sigint_handler, SIGINT))
        error_exit("Setting SIGINT handler:");

    server_work((uint16_t)port, timestep);

    return EXIT_SUCCESS;
}
