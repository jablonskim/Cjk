#ifndef _VEHICLE_LIST_H
#define _VEHICLE_LIST_H

#include "utils.h"

struct position_list
{
    int num_positions;
    struct position *position;
};

struct single_vehicle
{
    uint32_t id;
    uint16_t port;
    char *addr;
    struct position_list positions;
    struct single_vehicle *next;
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
};

uint32_t l_add_vehicle(struct vehicle_list *list, uint16_t port, char *addr);
int l_delete_vehicle(uint32_t id);
void init_vehicles_list(struct vehicle_list *list);
void free_vehicles_list(struct vehicle_list *list);
struct single_vehicle *get_position(struct single_vehicle *vehicle, int32_t *x, int32_t *y);

#endif
