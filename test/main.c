#include <stdio.h>

extern void server_info(const char *ip, unsigned short port);

int main(int argc, char *argv[])
{
    server_info("119.8.44.64", 40000);

    return 0;
}
