#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

extern void *get_in_addr(struct addrinfo *sa);
extern unsigned short get_in_port(struct addrinfo *sa);
extern void printable_in_addr(int af, void *sa, char *addr, size_t addrlen);
extern void printable_to_addr(int af, char *addr, void *sa);

void prepare_server(unsigned short port)
{
    char a_port[8] = {0};
    sprintf(a_port, "%d", port);

    struct addrinfo hints;
    struct addrinfo *serverinfo, *p;
    int sockstatus;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ( (sockstatus = getaddrinfo(NULL, a_port, &hints, &serverinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(sockstatus));
        exit(EXIT_FAILURE);
    }

    /* process serverinfo */

    freeaddrinfo(serverinfo);
}

void server_info(const char *ip, unsigned short port)
{
    char a_port[8] = {0};
    sprintf(a_port, "%d", port);

    int sockstatus;
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;

    if ( (sockstatus = getaddrinfo(ip, a_port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(sockstatus));
        return;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        char a_ip[INET6_ADDRSTRLEN] = {0};
        printable_in_addr(p->ai_family, get_in_addr(p), a_ip, sizeof a_ip);
        fprintf(stdout, "Server info: \n"
                "\tIP: %s\n"
                "\tport: %d\n",
                a_ip, get_in_port(p));

        memset(a_ip, 0, sizeof a_ip);
        struct sockaddr sa;
        printable_to_addr(AF_INET, "101.132.174.171", &sa);
        printable_in_addr(AF_INET, &sa, a_ip, sizeof a_ip);
        fprintf(stdout, "%s\n", a_ip);
    }

    freeaddrinfo(servinfo);
}
