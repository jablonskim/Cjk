#ifndef _SERVER_H
#define _SERVER_H

#include "utils.h"

struct background_work_data
{
    int step;
    struct vehicle_list *list;
};

struct get_coords_data
{
    struct vehicle_list *list;
    pthread_t thread;
    pthread_mutex_t mutex;
};

struct accept_connection_data
{
    int fd;
    pthread_mutex_t mutex;
    struct vehicle_list *vehicles;
    pthread_t thread;
};

struct calculate_distance_data
{
    uint32_t id;
    pthread_mutex_t mutex;
    pthread_t thread;
    struct vehicle_list *vehicles;
};

#endif
