#include "utils.h"
#include "vehicle_list.h"

void l_add_vehicle_to_list(struct vehicle_list *list, struct single_vehicle *v)
{
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
}

uint32_t l_add_vehicle(struct vehicle_list *list, uint16_t port, char *addr)
{
    uint32_t id;
    struct single_vehicle *v = (struct single_vehicle*)malloc(sizeof(struct single_vehicle));

    if(v == NULL)
        error_exit("Memory allocation:");

    t_start_writing(&list->mutex, &list->cond, &list->reading_threads);

    v->port = port;
    v->id = ++(list->last_id);
    v->addr = (char*)malloc(strlen(addr) + 1);

    if(v->addr == NULL)
        error_exit("Memory allocation:");
    
    memset(v->addr, 0, strlen(addr) + 1);

    strncpy(v->addr, addr, strlen(addr));
    v->next = NULL;
    v->positions.num_positions = 0;
    v->positions.position = NULL;

    l_add_vehicle_to_list(list, v);
    
    v->reading_threads = 0;
    t_check_error(pthread_mutex_init(&v->mutex, NULL));
    t_check_error(pthread_cond_init(&v->cond, NULL));

    id = v->id;

    t_stop_writing(&list->mutex);

    return id;
}

void l_delete_vehicle_p(struct vehicle_list *plist, struct single_vehicle *pv, struct single_vehicle *pprev)
{
    struct position *pp, *pt;

    free(pv->addr); 
    pp = pv->positions.position;
    while(pp != NULL)
    {
        pt = pp;
        pp = pp->next;
        free(pt);
    }

    if(pprev != NULL)
        pprev->next = pv->next;
    else
        plist->vehicle = pv->next;

    if(pv->next == NULL)
        plist->last = pprev;
        
    (plist->num_vehicles)--;
    
    t_check_error(pthread_mutex_destroy(&pv->mutex));
    t_check_error(pthread_cond_destroy(&pv->cond));

    free(pv);
}

int l_delete_vehicle(struct vehicle_list *list, uint32_t id)
{
    struct single_vehicle *v, *prev = NULL;
    v = list->vehicle;

    t_start_writing(&list->mutex, &list->cond, &list->reading_threads);

    while(v != NULL)
    {
        if(v->id == id)
        {
            l_delete_vehicle_p(list, v, prev);
            t_check_error(pthread_mutex_unlock(&list->mutex));

            return 1;
        }

        prev = v;
        v = v->next;
    }

    t_stop_writing(&list->mutex);
    
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
    list->longest.reading_threads = 0;
    list->longest.calc = NULL;
    list->longest.last = NULL;
    list->longest.last_id = 0;
    t_check_error(pthread_mutex_init(&list->longest.mutex, NULL));
    t_check_error(pthread_cond_init(&list->longest.cond, NULL));
}

void l_free_calculations_list(struct calculation *pc, pthread_mutex_t *pmutex, pthread_cond_t *pcond)
{
    while(pc != NULL)
    {
        struct calculation *poldc = pc;
        pc = pc->next;
        free(poldc);
    }

    t_check_error(pthread_mutex_destroy(pmutex));
    t_check_error(pthread_cond_destroy(pcond));
}

void free_vehicles_list(struct vehicle_list *list)
{
    struct single_vehicle *tmp, *p = list->vehicle;
    struct position *pos_tmp, *r;

    t_start_writing(&list->mutex, &list->cond, &list->reading_threads);
    l_free_calculations_list(list->longest.calc, &list->longest.mutex, &list->longest.cond);

    while(p != NULL)
    {
        free(p->addr);

        r = p->positions.position;

        while(r != NULL)
        {
            pos_tmp = r;
            r = r->next;
            free(pos_tmp);
        }

        tmp = p;
        p = p->next;
        free(tmp);
    }

    list->vehicle = NULL;
    list->last = NULL;

    t_stop_writing(&list->mutex);
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
        t_stop_reading(&list->mutex, &list->cond, &list->reading_threads);

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

    t_start_writing(&v->mutex, &v->cond, &v->reading_threads);

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

    t_stop_writing(&v->mutex);

    printf("New coords: %d %d\n", x, y);
}

uint32_t begin_getting_history(struct vehicle_list *list, uint32_t id, struct single_vehicle **v)
{
    struct single_vehicle *veh;

    t_start_reading(&list->mutex, &list->reading_threads);

    veh = list->vehicle;

    while(veh != NULL && veh->id != id)
        veh = veh->next;

    *v = veh;

    if(*v == NULL)
        return 0;

    t_start_reading(&(*v)->mutex, &(*v)->reading_threads);

    return (*v)->positions.num_positions;
}

void end_getting_history(struct vehicle_list *list, struct single_vehicle *v)
{
    if(v != NULL)
        t_stop_reading(&v->mutex, &v->cond, &v->reading_threads);

    t_stop_reading(&list->mutex, &list->cond, &list->reading_threads);
}

struct position *get_positions(struct single_vehicle *v)
{
    return v->positions.position;
}

struct position *get_next_position(struct position *pos)
{
    if(pos == NULL)
        return NULL;

    return pos->next;
}

uint32_t add_new_calculation(struct vehicle_list *list)
{
    uint32_t id;

    struct calculation *c = (struct calculation*)malloc(sizeof(struct calculation));
    if(c == NULL)
        error_exit("Allocating memory:");
    c->finished = 0;
    c->next = NULL;

    t_start_reading(&list->mutex, &list->reading_threads);
    t_start_writing(&list->longest.mutex, &list->longest.cond, &list->longest.reading_threads);

    c->id = ++(list->longest.last_id);
    if(list->longest.last == NULL)
    {
        list->longest.last = c;
        list->longest.calc = c;
    }
    else
    {
        list->longest.last->next = c;
        list->longest.last = c;
    }

    id = c->id;
    
    t_stop_writing(&list->longest.mutex);
    t_stop_reading(&list->mutex, &list->cond, &list->reading_threads);

    return id;
}

char get_calculation_status(struct vehicle_list *list, uint32_t id, uint32_t *winner)
{
    struct calculation *pcalc;
    char ret;

    t_start_reading(&list->mutex, &list->reading_threads);
    t_start_reading(&list->longest.mutex, &list->longest.reading_threads);

    pcalc = list->longest.calc;

    while(pcalc != NULL && pcalc->id != id)
        pcalc = pcalc->next;

    if(pcalc == NULL)
        ret = 'e';
    else
    {
        ret = pcalc->finished ? 'd' : 'o';
        *winner = pcalc->winner;
    }

    t_stop_reading(&list->longest.mutex, &list->longest.cond, &list->longest.reading_threads);
    t_stop_reading(&list->mutex, &list->cond, &list->reading_threads);

    return ret;
}

double l_calculate_vehicle_dist(struct position *p)
{
    double tmp_dist = 0;

    while(p != NULL && p->next != NULL)
    {
        tmp_dist += sqrt((p->next->x - p->x) * (p->next->x - p->x) 
                + (p->next->y - p->y) * (p->next->y - p->y));
        p = p->next;
    }

    return tmp_dist;
}

void l_save_calculation(struct vehicle_list *plist, uint32_t id, uint32_t winner)
{
    struct calculation *pc;

    t_start_writing(&plist->longest.mutex, &plist->longest.cond, &plist->longest.reading_threads);

    pc = plist->longest.calc;
    while(pc != NULL && pc->id != id)
        pc = pc->next;

    if(pc != NULL)
    {
        pc->finished = 1;
        pc->winner = winner;
    }

    t_stop_writing(&plist->longest.mutex);
}

void l_calculate_distance(struct vehicle_list *list, uint32_t id)
{
    double max_distance = 0, tmp_dist;
    uint32_t winner = 0;
    struct single_vehicle *v;

    t_start_reading(&list->mutex, &list->reading_threads);

    v = list->vehicle;
    while(v != NULL)
    {
        t_start_reading(&v->mutex, &v->reading_threads);

        tmp_dist = l_calculate_vehicle_dist(v->positions.position);
        
        if(tmp_dist > max_distance)
        {
            max_distance = tmp_dist;
            winner = v->id;
        }

        t_stop_reading(&v->mutex, &v->cond, &v->reading_threads);
        v = v->next;
    }

    l_save_calculation(list, id, winner);
    
    t_stop_reading(&list->mutex, &list->cond, &list->reading_threads);
}
