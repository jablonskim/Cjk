#include "utils.h"

void usage(const char *filename)
{
    fprintf(stderr, "USAGE: %s port timestep\n", filename);
    fprintf(stderr, "\tport - port nasluchu.\n");
    fprintf(stderr, "\ttimestep - czas odpytywania [s].\n");

    exit(EXIT_FAILURE);
}

void *background_work(void *data)
{
    return NULL;
}

void *collect_coordinates(void *data)
{
    return NULL;
}

void wait_for_connections(uint16_t port)
{

}

void *accept_connection(void *data)
{
    return NULL;
}

void server_work(uint16_t port, int timestep)
{

    wait_for_connections(port);
}

int main(int argc, char **argv)
{
    int port, timestep;

    if(argc != 3)
        usage(argv[0]);

    port = atoi(argv[1]);
    timestep = atoi(argv[2]);

    if(port <= 0 || timestep <= 0)
        usage(argv[0]);

    if(sethandler(SIG_IGN, SIGPIPE))
        error_exit("Setting SIGPIPE handler:");

    server_work((uint16_t)port, timestep);

    return EXIT_SUCCESS;
}
