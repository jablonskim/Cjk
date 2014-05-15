#include "utils.h"

#define X_COORD_LIMIT 9000
#define Y_COORD_LIMIT 18000
#define MAX_STEP 2

volatile sig_atomic_t work = 1;

struct vehicle_data_container
{
    int32_t *x;
    int32_t *y;
    int step;
    pthread_mutex_t mutex;
};

void usage(const char *filename)
{
    fprintf(stderr, "USAGE: %s port step\n", filename);
    fprintf(stderr, "\tport - Port nasluchu.\n");
    fprintf(stderr, "\tstep - Przedzial czasu po jakim nastepuje przesuniecie.\n"); 

    exit(EXIT_FAILURE);
}

void sigint_handler(int sig)
{
    work = 0;
}

void *moving(void *arg)
{
    int x, y;

    struct vehicle_data_container *vehicle_data = (struct vehicle_data_container*)arg;

    while(work)
    {
        safe_sleep(vehicle_data->step);

        x = rand() % (2 * MAX_STEP + 1) - MAX_STEP;
        y = rand() % (2 * MAX_STEP + 1) - MAX_STEP;

        t_check_error(pthread_mutex_lock(&(vehicle_data->mutex)));
        *vehicle_data->x += x;
        *vehicle_data->y += y;
        printf("Nowa lokacja: [%d; %d]\n", *vehicle_data->x, *vehicle_data->y);
        t_check_error(pthread_mutex_unlock(&(vehicle_data->mutex)));
    }

    return NULL;
}

void send_coordinates(int sockfd, int32_t *x, int32_t *y, pthread_mutex_t *mutex)
{
    int32_t buf[2];
    t_check_error(pthread_mutex_lock(mutex));
    buf[0] = htonl(*x);
    buf[1] = htonl(*y);
    t_check_error(pthread_mutex_unlock(mutex));
    socket_write(sockfd, (char*)buf, 2 * sizeof(int32_t));
}

void data_provider(int32_t *x, int32_t *y, int16_t port, pthread_mutex_t *mutex)
{
    int sockfd, ssocket;

    sockfd = make_and_bind_socket(port, SOCK_STREAM);
    socket_listen(sockfd, 1);

    while(work)
    {
        ssocket = 0;
        while(work && (ssocket = accept(sockfd, NULL, NULL)) < 0)
        {
            if(errno == EINTR)
                continue;

            error_exit("Accepting connection:");
        }

        if(work)
            send_coordinates(ssocket, x, y, mutex);

        if(ssocket > 0 && TEMP_FAILURE_RETRY(close(ssocket)) < 0)
            error_exit("Closing socket:");
    }

    if(TEMP_FAILURE_RETRY(close(sockfd)) < 0)
        error_exit("Closing socket:");
}

void vehicle_work(int port, int step)
{
    int32_t coordinates[2];
    pthread_t bgthread;

    coordinates[0] = (rand() % (2 * X_COORD_LIMIT + 1)) - X_COORD_LIMIT;
    coordinates[1] = (rand() % (2 * Y_COORD_LIMIT + 1)) - Y_COORD_LIMIT;

    struct vehicle_data_container vehicle_data;
    vehicle_data.x = &coordinates[0];
    vehicle_data.y = &coordinates[1];
    vehicle_data.step = step;
    t_check_error(pthread_mutex_init(&(vehicle_data.mutex), NULL));
    
    t_check_error(pthread_create(&bgthread, NULL, moving, &vehicle_data));
    data_provider(&coordinates[0], &coordinates[1], (int16_t)port, &(vehicle_data.mutex));
    printf("Oczekiwanie na zamkniecie...\n");
    t_check_error(pthread_join(bgthread, NULL));
    t_check_error(pthread_mutex_destroy(&(vehicle_data.mutex)));
}

int main(int argc, char **argv)
{
    int port, step;

    if(argc != 3)
        usage(argv[0]);

    port = atoi(argv[1]);
    step = atoi(argv[2]);

    if(port <= 0 || step <= 0)
        usage(argv[0]);

    srand(time(NULL) ^ getpid());

    if(sethandler(SIG_IGN, SIGPIPE))
        error_exit("Setting SIGPIPE handler:");
    if(sethandler(sigint_handler, SIGINT))
        error_exit("Setting SIGINT handler:");

    vehicle_work(port, step);

    return EXIT_SUCCESS;
}
