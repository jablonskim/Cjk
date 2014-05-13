#include "utils.h"
#include "vehicle_list.h"

uint32_t l_add_vehicle(struct vehicle_list *list, uint16_t port, char *addr)
{
    struct single_vehicle *v = (struct single_vehicle*)malloc(sizeof(struct single_vehicle));

    if(v == NULL)
        error_exit("Memory allocation:");

    v->port = port;
    v->id = ++(list->last_id);
    v->addr = (char*)malloc(strlen(addr) + 1);

    if(v->addr == NULL)
        error_exit("Memory allocation:");

    strncpy(v->addr, addr, strlen(addr));
    v->next = NULL;
    v->positions.num_positions = 0;
    v->positions.position = NULL;

    if(list->last == NULL)
    {
        list->last = v;
        list->vehicle = v;
    }
    else
    {
        list->last->next = v;
        list->last = v;
    }

    (list->num_vehicles)++;

    return v->id;
}

int l_delete_vehicle(uint32_t id)
{
    return 0;
}

void init_vehicles_list(struct vehicle_list *list)
{
    list->num_vehicles = 0;
    list->last_id = 0;
    list->vehicle = NULL;
    list->last = NULL;
}

void free_vehicles_list(struct vehicle_list *list)
{
}

struct single_vehicle *get_position(struct single_vehicle *vehicle, int32_t *x, int32_t *y)
{
    return NULL;
}
