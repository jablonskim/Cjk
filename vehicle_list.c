#include "utils.h"
#include "vehicle_list.h"

uint32_t l_add_vehicle(struct vehicle_list *list, uint16_t port, char *addr)
{
    uint32_t id;
    struct single_vehicle *v = (struct single_vehicle*)malloc(sizeof(struct single_vehicle));

    if(v == NULL)
        error_exit("Memory allocation:");

    t_check_error(pthread_mutex_lock(&list->mutex));
    while(list->reading_threads != 0)
        t_check_error(pthread_cond_wait(&list->cond, &list->mutex));

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

    v->reading_threads = 0;
    t_check_error(pthread_mutex_init(&v->mutex, NULL));
    t_check_error(pthread_cond_init(&v->cond, NULL));

    id = v->id;

    t_check_error(pthread_mutex_unlock(&list->mutex));

    return id;
}

int l_delete_vehicle(struct vehicle_list *list, uint32_t id)
{
    struct single_vehicle *v, *prev = NULL;
    struct position *p, *t;
    v = list->vehicle;

    t_check_error(pthread_mutex_lock(&list->mutex));
    while(list->reading_threads != 0)
        t_check_error(pthread_cond_wait(&list->cond, &list->mutex));

    while(v != NULL)
    {
        if(v->id == id)
        {
            free(v->addr);
            p = v->positions.position;
            while(p != NULL)
            {
                t = p;
                p = p->next;
                free(t);
            }

            if(prev != NULL)
                prev->next = v->next;
            else
                list->vehicle = v->next;

            if(v->next == NULL)
                list->last = prev;

            (list->num_vehicles)--;

            t_check_error(pthread_mutex_destroy(&v->mutex));
            t_check_error(pthread_cond_destroy(&v->cond));

            free(v);

            t_check_error(pthread_mutex_unlock(&list->mutex));

            return 1;
        }

        prev = v;
        v = v->next;
    }

    t_check_error(pthread_mutex_unlock(&list->mutex));
    
    return 0;
}

void init_vehicles_list(struct vehicle_list *list)
{
    list->num_vehicles = 0;
    list->last_id = 0;
    list->vehicle = NULL;
    list->last = NULL;
    list->reading_threads = 0;
    t_check_error(pthread_mutex_init(&list->mutex, NULL));
    t_check_error(pthread_cond_init(&list->cond, NULL));
}

void free_vehicles_list(struct vehicle_list *list)
{
    struct single_vehicle *p = list->vehicle;

    t_check_error(pthread_mutex_lock(&list->mutex));
    while(list->reading_threads != 0)
        t_check_error(pthread_cond_wait(&list->cond, &list->mutex));

    while(p != NULL)
    {
        free(p->addr);

        struct position *r = p->positions.position;

        while(r != NULL)
        {
            struct position *tmp2 = r;
            r = r->next;
            free(tmp2);
        }

        struct single_vehicle *tmp = p;
        p = p->next;
        free(tmp);
    }

    list->vehicle = NULL;
    list->last = NULL;

    t_check_error(pthread_mutex_unlock(&list->mutex));
    t_check_error(pthread_mutex_destroy(&list->mutex));
    t_check_error(pthread_cond_destroy(&list->cond));
}

struct single_vehicle *get_vehicles(struct vehicle_list *list)
{
    t_check_error(pthread_mutex_lock(&list->mutex));
    if(list->vehicle != NULL)
        (list->reading_threads)++;
    t_check_error(pthread_mutex_unlock(&list->mutex));

    return list->vehicle;
}

struct single_vehicle *get_next_vehicle(struct vehicle_list *list, struct single_vehicle *v)
{
    struct single_vehicle *veh;

    if(v == NULL)
        return NULL;

    veh = v->next;

    if(veh == NULL)
    {
        t_check_error(pthread_mutex_lock(&list->mutex));
        (list->reading_threads)--;
        t_check_error(pthread_cond_broadcast(&list->cond));
        t_check_error(pthread_mutex_unlock(&list->mutex));
    }

    return veh;
}

void add_coords(int32_t x, int32_t y, struct single_vehicle *v)
{
    struct position *p = (struct position*)malloc(sizeof(struct position));
    if(p == NULL)
        error_exit("Allocating memory:");

    p->next = NULL;
    p->x = x;
    p->y = y;

    t_check_error(pthread_mutex_lock(&v->mutex));
    while(v->reading_threads != 0)
        t_check_error(pthread_cond_wait(&v->cond, &v->mutex));

    if(v->positions.position == NULL)
    {
        v->positions.position = p;
        v->positions.last = p;
    }
    else
    {
        v->positions.last->next = p;
        v->positions.last = p;
    }

    (v->positions.num_positions)++;

    t_check_error(pthread_mutex_unlock(&v->mutex));

    printf("New coords: %d %d\n", x, y);
}
