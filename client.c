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
    
    if(nump != 2 || (iport = atoi(params[0])) <= 0)
    {
        socket_close(fd);
        usage("client");
    }

    port = htons((uint16_t)iport);
    data_size = htonl(strlen(params[1]));

    if(socket_write(fd, "r", 1) != 1) error_exit("Sending data:");
    if(socket_write(fd, &port, sizeof(uint16_t)) != sizeof(uint16_t)) error_exit("Sending data:");
    if(socket_write(fd, &data_size, sizeof(uint32_t)) != sizeof(uint32_t)) error_exit("Sending data:");
    if(socket_write(fd, params[1], strlen(params[1])) != strlen(params[1])) error_exit("Sending data:");

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
}

void calculate_distance(int fd, int nump, char **params)
{
}

void check_status(int fd, int nump, char **params)
{
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
