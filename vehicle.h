#ifndef _VEHICLE_H
#define _VEHICLE_H

#include "utils.h"

struct vehicle_data_container
{
    int32_t *x;
    int32_t *y;
    int step;
    pthread_mutex_t mutex;
};

#endif
