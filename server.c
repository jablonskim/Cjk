#include "utils.h"
#include "vehicle_list.h"
#include "server.h"

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

void *collect_coordinates(void *data)
{
    int32_t coords[2];
    int fd;
    struct single_vehicle *v;
    struct get_coords_data *d = (struct get_coords_data*)data;

    v = get_vehicles(d->list);

    while(v)
    {
        fd = make_socket(SOCK_STREAM);
        socket_connect(fd, v->addr, v->port);

        if(socket_read(fd, coords, 2 * sizeof(int32_t)) == 2 * sizeof(int32_t))
        {
            add_coords(ntohl(coords[0]), ntohl(coords[1]), v);
        }

        socket_close(fd);

        v = get_next_vehicle(d->list, v);
    }

    stop_detached_thread(&d->mutex, d);
    
    return NULL;
}

void *background_work(void *data)
{
    pthread_attr_t t_attr;

    struct get_coords_data *cd;
    struct background_work_data *bgwd = (struct background_work_data*)data;

    while(work)
    {
        safe_sleep(bgwd->step, NULL);

        cd = malloc(sizeof(struct get_coords_data));
        if(cd == NULL)
            error_exit("Allocating memory:");
        cd->list = bgwd->list;

        start_detached_thread(&cd->mutex, &t_attr, &cd->thread, collect_coordinates, cd);
    }

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
    
    printf("Vehicle %d registered. Count = %d\n", ntohl(id), vehicles->num_vehicles);
    free(addr);
}

void delete_vehicle(int fd, struct vehicle_list *vehicles)
{
    uint32_t id;
    char status;

    if(socket_read(fd, &id, sizeof(uint32_t)) != sizeof(uint32_t)) return;
    id = ntohl(id);

    status = l_delete_vehicle(vehicles, id) ? 'k' : 'e';
    if(socket_write(fd, &status, 1) != 1) return;
    printf(status == 'k' ? "Pojazd %d zostal usuniety. Count = %d\n" : 
            "Pojazd %d nie zostal usuniety. Count = %d\n", id, vehicles->num_vehicles);
}

void get_history(int fd, struct vehicle_list *vehicles)
{
    int32_t id, count, x, y;
    struct single_vehicle *v;
    struct position *p;

    if(socket_read(fd, &id, sizeof(uint32_t)) != sizeof(uint32_t)) return;
    id = ntohl(id);

    count = begin_getting_history(vehicles, id, &v);
    count = htonl(count);
    
    if(socket_write(fd, &count, sizeof(uint32_t)) == sizeof(uint32_t) && v != NULL)
    {
        p = get_positions(v);
        
        while(p != NULL)
        {
            x = htonl(p->x);
            y = htonl(p->y);

            if(socket_write(fd, &x, sizeof(int32_t)) != sizeof(int32_t)) break;
            if(socket_write(fd, &y, sizeof(int32_t)) != sizeof(int32_t)) break;
              
            p = get_next_position(p);
        }
    }

    end_getting_history(vehicles, v);
}

void *calculate_distance_bg(void *data)
{
    struct calculate_distance_data *d = (struct calculate_distance_data*)data;

    l_calculate_distance(d->vehicles, d->id);
    stop_detached_thread(&d->mutex, d);

    return NULL;
}

void calculate_distance(int fd, struct vehicle_list *vehicles)
{
    uint32_t id;
    pthread_attr_t t_attr;
    struct calculate_distance_data *data;

    data = (struct calculate_distance_data*)malloc(sizeof(struct calculate_distance_data));
    if(data == NULL)
        error_exit("Allocating memory:");

    data->vehicles = vehicles;
    id = add_new_calculation(vehicles);
    data->id = id;
    id = htonl(id);

    if(socket_write(fd, &id, sizeof(uint32_t)) == sizeof(uint32_t))
        start_detached_thread(&data->mutex, &t_attr, &data->thread, calculate_distance_bg, data);
    else
        free(data);
}

void check_status(int fd, struct vehicle_list *vehicles)
{
    uint32_t id, winner;
    char status;

    if(socket_read(fd, &id, sizeof(uint32_t)) != sizeof(uint32_t)) return;
    id = ntohl(id);

    status = get_calculation_status(vehicles, id, &winner);
    if(socket_write(fd, &status, 1) != 1) return;
    
    if(status == 'd')
    {
        winner = htonl(winner);
        socket_write(fd, &winner, sizeof(uint32_t));
    }
}

void *accept_connection(void *data)
{
    char command;
    struct accept_connection_data *d = (struct accept_connection_data*)data;

    if(socket_read(d->fd, &command, 1) == 1)
    {
        switch(command)
        {
            case 'r':
                register_vehicle(d->fd, d->vehicles);
                break;
            case 'd':
                delete_vehicle(d->fd, d->vehicles);
                break;
            case 'h':
                get_history(d->fd, d->vehicles);
                break;
            case 't':
                calculate_distance(d->fd, d->vehicles);
                break;
            case 's':
                check_status(d->fd, d->vehicles);
                break;
        }
    }

    socket_close(d->fd);
    stop_detached_thread(&d->mutex, d);    

    return NULL;
}

void wait_for_connections(uint16_t port, struct vehicle_list *vehicles)
{
    int sockfd;
    struct accept_connection_data *data;
    pthread_attr_t t_attr;

    t_sigmask(SIGINT, SIG_UNBLOCK);
    sockfd = make_and_bind_socket(port, SOCK_STREAM);
    socket_listen(sockfd, 16);

    while(work)
    {
        data = (struct accept_connection_data*)malloc(sizeof(struct accept_connection_data));
        if(data == NULL) error_exit("Memory allocation:");
        data->fd = 0;
        data->vehicles = vehicles;
        
        while(work && (data->fd = accept(sockfd, NULL, NULL)) < 0)
        {
            if(errno == EINTR)
                continue;
            error_exit("Accepting connection:");
        }

        t_sigmask(SIGINT, SIG_BLOCK);
        if(data->fd > 0)
            start_detached_thread(&data->mutex, &t_attr, &data->thread, accept_connection, data);
        else
            free(data);
        t_sigmask(SIGINT, SIG_UNBLOCK);
    }

    socket_close(sockfd);
}

void server_work(uint16_t port, int timestep)
{
    pthread_t bgthread;
    struct background_work_data bgwd;
    struct vehicle_list vehicles;

    init_vehicles_list(&vehicles);

    bgwd.step = timestep;
    bgwd.list = &vehicles;

    t_check_error(pthread_create(&bgthread, NULL, background_work, &bgwd));
    wait_for_connections(port, &vehicles);

    printf("Oczekiwanie na zamkniecie...\n");
    t_check_error(pthread_join(bgthread, NULL));
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

    t_sigmask(SIGINT, SIG_BLOCK);

    server_work((uint16_t)port, timestep);

    return EXIT_SUCCESS;
}
