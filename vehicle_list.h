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

struct calculation
{
    uint32_t id;
    int finished;
    uint32_t winner;
    struct calculation *next;
};

struct longest_list
{
    int last_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int reading_threads;
    struct calculation *calc;
    struct calculation *last;
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
    struct longest_list longest;
};

uint32_t l_add_vehicle(struct vehicle_list *list, uint16_t port, char *addr);
int l_delete_vehicle(struct vehicle_list *list, uint32_t id);
void init_vehicles_list(struct vehicle_list *list);
void free_vehicles_list(struct vehicle_list *list);
struct single_vehicle *get_vehicles(struct vehicle_list *list);
struct single_vehicle *get_next_vehicle(struct vehicle_list *list, struct single_vehicle *v);
void add_coords(int32_t x, int32_t y, struct single_vehicle *v);
uint32_t begin_getting_history(struct vehicle_list *list, uint32_t id, struct single_vehicle **v);
void end_getting_history(struct vehicle_list *list, struct single_vehicle *v);
struct position *get_positions(struct single_vehicle *v);
struct position *get_next_position(struct position *pos);
uint32_t add_new_calculation(struct vehicle_list *list);
char get_calculation_status(struct vehicle_list *list, uint32_t id, uint32_t *winner);
void l_calculate_distance(struct vehicle_list *list, uint32_t id);

#endif
