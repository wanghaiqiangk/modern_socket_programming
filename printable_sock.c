#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

void *get_in_addr(struct addrinfo *sa)
{
    switch (sa->ai_family)
    {
        case AF_INET:
            return (struct sockaddr_in *)sa->ai_addr;
        case AF_INET6:
            return (struct sockaddr_in6 *)sa->ai_addr;
        default:
            fprintf(stdout, "Not implemented yet\n");
    }
    return NULL;
}

unsigned short get_in_port(struct addrinfo *sa)
{
    switch (sa->ai_family)
    {
        case AF_INET:
            return ntohs(((struct sockaddr_in *)sa->ai_addr)->sin_port);
        case AF_INET6:
            return ntohs(((struct sockaddr_in6 *)sa->ai_addr)->sin6_port);
        default:
            fprintf(stdout, "Not implemented yet\n");
    }
    return 0;
}

void printable_in_addr(int af, void *sa, char *addr, size_t addrlen)
{
    if (addr == NULL)
    {
        fprintf(stderr, "3rd parameter is null\n");
        return;
    }

    if (addrlen < INET_ADDRSTRLEN)
    {
        fprintf(stderr, "address buffer too small\n");
        return;
    }

    if (af == AF_INET6 && addrlen < INET6_ADDRSTRLEN)
    {
        fprintf(stderr, "address buffer cannot hold IPv6 lenght\n");
        return;
    }

    void *ia = NULL;
    if (af == AF_INET)
    {
        ia = &((struct sockaddr_in *)sa)->sin_addr;
    }
    else
    {
        ia = &((struct sockaddr_in6 *)sa)->sin6_addr;
    }

    inet_ntop(af, ia, addr, addrlen);
}

void printable_to_addr(int af, char *addr, void *sa)
{
    if (addr == NULL)
    {
        fprintf(stderr, "2nd parameter is null\n");
        return;
    }

    void *ia = NULL;
    if (af == AF_INET)
    {
        ia = &((struct sockaddr_in *)sa)->sin_addr;
    }
    else
    {
        ia = &((struct sockaddr_in6 *)sa)->sin6_addr;
    }

    inet_pton(af, addr, ia);
}
