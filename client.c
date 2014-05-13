#include "utils.h"

void usage(const char *filename)
{
    fprintf(stderr, "USAGE: %s server port command [dodatkowe parametry]\n", filename);
    fprintf(stderr, "\tserver  - Adres serwera.\n");
    fprintf(stderr, "\tport    - Port nasluchu serwera.\n");
    fprintf(stderr, "\tcommand - Polecenie do wykonania\n\n");

    fprintf(stderr, "Mozliwe polecenia z dodatkowymi parametrami:\n");
    fprintf(stderr, "\tr address port\n");
    fprintf(stderr, "\t\tRejestracja nowego pojazdu. \n\t\taddress - adres pojazdu, port - port nasluchu pojazdu\n");
    fprintf(stderr, "\t\tZwraca id przypisane do nowego pojazdu.\n");
    fprintf(stderr, "\td id\n");
    fprintf(stderr, "\t\tWyrejestrowanie pojazdu. id - id pojazdu.\n");
    fprintf(stderr, "\th id\n");
    fprintf(stderr, "\t\tPobranie historii trasy pojazdu. id - id pojazdu.\n");
    fprintf(stderr, "\tt\n");
    fprintf(stderr, "\t\tZlecenie obliczenia, ktory pojazd przejechal najdluzsza trase.\n");
    fprintf(stderr, "\t\tZwraca id potrzebne do sprawdzenia wynikow.\n");
    fprintf(stderr, "\ts id\n");
    fprintf(stderr, "\t\tSprawdzenie statusu i ewentualne pobranie wynikow obliczen o okreslonym id.\n");

    exit(EXIT_FAILURE);
}

void register_vehicle(int fd, int nump, char **params)
{
    int iport;
    uint16_t port;
    uint32_t data_size, id;
    char status;
    
    if(nump != 2 || (iport = atoi(params[1])) <= 0)
    {
        socket_close(fd);
        usage("client");
    }

    port = htons((uint16_t)iport);
    data_size = htonl(strlen(params[0]));

    if(socket_write(fd, "r", 1) != 1) error_exit("Sending data:");
    if(socket_write(fd, &port, sizeof(uint16_t)) != sizeof(uint16_t)) error_exit("Sending data:");
    if(socket_write(fd, &data_size, sizeof(uint32_t)) != sizeof(uint32_t)) error_exit("Sending data:");
    if(socket_write(fd, params[0], strlen(params[0])) != strlen(params[0])) error_exit("Sending data:");

    if(socket_read(fd, &status, 1) != 1) error_exit("Receiving data:");
    if(status != 'k') {
        printf("Blad! Pojazd nie moze byc zarejestrowany.\n");
        return;
    }
    if(socket_read(fd, &id, sizeof(uint32_t)) != sizeof(uint32_t)) error_exit("Receiving data:");
    id = ntohl(id);

    printf("ID = %d\n", id);
}

void delete_vehicle(int fd, int nump, char **params)
{
    int id;
    uint32_t iddata;
    char status;

    if(nump != 1 || (id = atoi(params[0])) <= 0) 
    {
        socket_close(fd);
        usage("client");
    }

    iddata = htonl(id);
    
    if(socket_write(fd, "d", 1) != 1) error_exit("Sending data:");
    if(socket_write(fd, &iddata, sizeof(uint32_t)) != sizeof(uint32_t)) error_exit("Sending data:");

    if(socket_read(fd, &status, 1) != 1) error_exit("Receiving data:");

    printf(status == 'k' ? "Pojazd zostal wyrejestrowany.\n" : "Blad! Nie mozna wyrejestrowac pojazdu.\n");
}

void get_history(int fd, int nump, char **params)
{
    uint32_t id, count;
    int32_t coords[2];
    int i;

    if(nump != 1 || (id = atoi(params[0])) <= 0)
    {
        socket_close(fd);
        usage("client");
    }

    id = htonl(id);

    if(socket_write(fd, "h", 1) != 1) error_exit("Sending data:");
    if(socket_write(fd, &id, sizeof(uint32_t)) != sizeof(uint32_t)) error_exit("Sending data:");

    if(socket_read(fd, &count, sizeof(uint32_t)) != sizeof(uint32_t)) error_exit("Receiving data:");
    count = ntohl(count);
    printf("W historii pojazdu %d jest %d lokacji:\n", ntohl(id), count);

    for(i = 0; i < count; ++i)
    {
        if(socket_read(fd, coords, 2 * sizeof(int32_t)) != sizeof(int32_t)) error_exit("Receiving data:");
        printf("[%d; %d]\n", ntohl(coords[0]), ntohl(coords[1]));
    }
}

void calculate_distance(int fd, int nump, char **params)
{
    uint32_t id;

    if(socket_write(fd, "t", 1) != 1) error_exit("Sending data:");

    if(socket_read(fd, &id, sizeof(uint32_t)) != sizeof(uint32_t)) error_exit("Receiving data:");
    id = ntohl(id);

    printf("Status ID = %d\n", id);
}

void check_status(int fd, int nump, char **params)
{
    int id;
    uint32_t iid;
    char status;

    if(nump != 1 || (id = atoi(params[0])) <= 0)
        usage("client");

    iid = htonl(id);

    if(socket_write(fd, "s", 1) != 1) error_exit("Sending data:");
    if(socket_write(fd, &iid, sizeof(uint32_t)) != sizeof(uint32_t)) error_exit("Sending data:");

    if(socket_read(fd, &status, 1) != 1) error_exit("Receiving data:");

    switch(status)
    {
        case 'o':
            printf("Trwaja obliczenia, sprobuj pozniej.\n");
            break;

        case 'd':
            if(socket_read(fd, &iid, sizeof(uint32_t)) != sizeof(uint32_t)) error_exit("Receiving data:");
            iid = ntohl(iid);
            printf("Najdluzsza trase przejechal pojazd %d.\n", iid);
            break;

        default:
            printf("Blad! Nie mozna odebrac statusu dla ID = %d\n", id);
            break;
    }
}

void client_work(char command, char *server_addr, uint16_t port, int nump, char **params)
{
    int fd;

    if(command != 'r' && command != 'd' && command != 'h' && command != 't' && command != 's')
        usage("client");

    fd = make_socket(SOCK_STREAM);
    socket_connect(fd, server_addr, port);

    switch(command)
    {
        case 'r':
            register_vehicle(fd, nump, params);
            break;

        case 'd':
            delete_vehicle(fd, nump, params);
            break;

        case 'h':
            get_history(fd, nump, params);
            break;

        case 't':
            calculate_distance(fd, nump, params);
            break;

        case 's':
            check_status(fd, nump, params);
            break;
    }

    socket_close(fd);
}

int main(int argc, char **argv)
{
    char command;
    int port;

    if(argc < 4)
        usage(argv[0]);
    
    command = argv[3][0];
    port = atoi(argv[2]);

    if(port <= 0)
        usage(argv[0]);

    if(sethandler(SIG_IGN, SIGPIPE))
        error_exit("Setting SIGPIPE handler:");

    client_work(command, argv[1], (uint16_t)port, argc - 4, argv + 4);

    return EXIT_SUCCESS;
}
