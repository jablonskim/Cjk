#ifndef _VEHICLE_LIST_H
#define _VEHICLE_LIST_H

#include "utils.h"

struct position_list
{
    int num_positions;
    struct position *position;
    struct position *last;
};

struct single_vehicle
{
    uint32_t id;
    uint16_t port;
    char *addr;
    struct position_list positions;
    struct single_vehicle *next;
    int reading_threads;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

struct position
{
    int32_t x;
    int32_t y;
    struct position *next;
};

struct vehicle_list
{
    int num_vehicles;
    int last_id;
    struct single_vehicle *vehicle;
    struct single_vehicle *last;
    int reading_threads;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

uint32_t l_add_vehicle(struct vehicle_list *list, uint16_t port, char *addr);
int l_delete_vehicle(struct vehicle_list *list, uint32_t id);
void init_vehicles_list(struct vehicle_list *list);
void free_vehicles_list(struct vehicle_list *list);
struct single_vehicle *get_vehicles(struct vehicle_list *list);
struct single_vehicle *get_next_vehicle(struct vehicle_list *list, struct single_vehicle *v);
void add_coords(int32_t x, int32_t y, struct single_vehicle *v);

#endif
