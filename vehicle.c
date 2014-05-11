#include "utils.h"

#define X_COORD_LIMIT 9000
#define Y_COORD_LIMIT 18000
#define MAX_STEP 2

void usage(const char *filename)
{
    fprintf(stderr, "USAGE: %s step\n", filename);
    fprintf(stderr, "\tstep - Przedzial czasu po jakim nastepuje przesuniecie.\n"); 

    exit(EXIT_FAILURE);
}

void moving()
{
}

void data_provider()
{
}

int main(int argc, char **argv)
{
    int x, y;

    srand(time(NULL) ^ getpid());
    x = (rand() % (2 * X_COORD_LIMIT + 1)) - X_COORD_LIMIT;
    y = (rand() % (2 * Y_COORD_LIMIT + 1)) - Y_COORD_LIMIT;

    return EXIT_SUCCESS;
}
