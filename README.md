# modern_socket_programming

The general paradigm of socket programming is not dramatically changed from the past to now. The modern way means how it can process IPv4 and IPv6.

Before, there are different APIs distinguished by suffix for IPv4 (if no apparent suffix) and IPv6 (if suffixed by digit number six). That means the programmer has to be aware of which version of IP address in use by the communicatee. As IPv6 is becoming more common, many sites offers both versions of IP address to identify themselves.

Therefore, a slight modern way is to equally treat both IPv4 and IPv6. Use the same APIs then choose the one if specific requirement for a particular version of IP address is demanded, otherwise choose either one as long as the communication can be established.

# get address information - modern way

[man - getaddrinfo](https://man7.org/linux/man-pages/man3/getaddrinfo.3.html)

## One Data Structure

```c
struct addrinfo {
	int              ai_flags;       // input
	int              ai_family;      // input or output
	int              ai_socktype;    // input or output
	int              ai_protocol;    // input or output
	socklen_t        ai_addrlen;     // output
	struct sockaddr *ai_addr;        // output
	char            *ai_canonname;   // output
	struct addrinfo *ai_next;        // next element in the linked list
};
```

At the first glance, you may notice the last member of `ai_next` as a pointer pointing to itself. This means `struct addrinfo` can be used as forward linked list. The idea is that both IPv4 and IPv6 information are stored together as a list. People can traverse the list to use the one which satisfy their requirements.

At the next glance, you may notice the third member from the last of `ai_addr` as type of `struct sockaddr`. `struct sockaddr` is an amazing structure that can hold either IPv4 or IPv6 address and can be casted to `struct sockaddr_in` or `struct sockaddr_in6` respectively. Inside it, IP address and port number are stored.

| Member      | Values                       | Explanation                                                  |
| ----------- | ---------------------------- | ------------------------------------------------------------ |
| ai_family   | AF_INET/AF_INET6/AF_UNSPEC   | Hint indicating which version of IP address to use. Suitable for the first parameter of `socket()` |
| ai_socktype | SOCK_STREAM/SOCK_DGRAM/0     | Hint TCP or UDP is in use.                                   |
| ai_protocol | 0                            | Suitable for the third parameter of `socket()`               |
| ai_flags    | bitwise OR (see FALGS table) |                                                              |

Above four members must be set when used as `hints`. Also any other members must be initialized as zero or null pointer.

| FLAGS          | Special                                                      | Explanation                                                  |
| -------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| AI_V4MAPPED    | Default value when `hints` is `NULL`                         | If `hints.ai_family` is `AF_INET6` but no matching IPv6 address is found, return IPv4-mapped IPv6 address. |
| AI_ALL         | Only valid when `AI_V4MAPPED` is specified                   | Return both IPv4-mapped IPv6 and IPv6 address                |
| AI_ADDRCONFIG  | Default value when `hints` is `NULL`                         | Return an IPv4 address only if the local system has at least one IPv4 address configured. Same for IPv6. Loopback address is not considered as a valid configured address. |
| AI_NUMERICHOST |                                                              | `node` must be numerical address string                      |
| AI_NUMERICSERV |                                                              | `service` must be numerical port string                      |
| AI_CANONNAME   | Official name is returned in the first element of linked list | Host's official name                                         |



## Three APIs

```c
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);

void freeaddrinfo(struct addrinfo *res);

# get error string from the return value of getaddrinfo()
const char *gai_strerror(int errcode);
```

- `node` can be
  - numerical address string, typically numbers-and-dots for IPv4 or hexadecimal string format for IPv6
  - hostname, which will be processed by host address lookup
- If `node` is `NULL` and `hints.ai_flags` contains `AI_PASSIVE`, the socket is suitable for server, then `bind()` and `accept()`
  - The returned socket address is wildcard address, typically `INADDR_ANY` for IPv4 or `IN6ADDR_ANY_INIT` for IPv6.

- If `hints.ai_flags` doesn't contain `AI_PASSIVE`, the socket is suitable for client, then `connect()`, `sendto()` or `sendfrom()`
  - if `node` is `NULL` too, the returned socket address is loopback address, typically `INADDR_LOOPBACK` for IPv4 or `IN6ADDR_LOOPBACK_INIT` for IPv6. Then the communication is desired to happen on the same host.



- `service` can be numerical port string or service name. This will be used to initialize port number
  - `service` can be `NULL`, leaving port number uninitialized.



- `res` is a linked list containing one or more socket address. Why there can be more than one address?
  - multihomed, accessible over multiple protocol, like both IPv4 and IPv6
  - service is available from multiple socket type, like stream and datagram
- `res` is ranked and ordered by `getaddrinfo()` internally. The ranking algorithm is specified by RFC3484 and can be tweaked by editing `/etc/gai.conf`.
  - The recommended way is to try in order and use the first one that satisfy your requirements.



## A Sample Usage

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

void make_server_info(short port)
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
```

# get address information - traditional way

## One Base Structure

Being "base", I mean it's like a class in C++ with inheritance feature.

The base structure is not strange to us, because we have already seen it - `struct sockaddr`. Its fields are:

```c
struct sockaddr {
	unsigned short sa_family; // address family, AF_xxx
	char sa_data[14]; // 14 bytes of protocol address
};
```

Usually, we do not directly use such structure, but instead use two "derived" children - `struct sockaddr_in` for IPv4 and `struct sockaddr_in6` for IPv6. The "base" and "derived" structures can cast to each other.

```c
struct sockaddr_in {
	short int sin_family; // Address family, AF_INET
	unsigned short int sin_port; // Port number
	struct in_addr sin_addr; // Internet address
	unsigned char sin_zero[8]; // Same size as struct sockaddr
};

struct in_addr {
	uint32_t s_addr; // that's a 32-bit int (4 bytes)
};

struct sockaddr_in6
	u_int16_t sin6_family; // address family, AF_INET6
	u_int16_t sin6_port; // port number, Network Byte Order
	u_int32_t sin6_flowinfo; // IPv6 flow information
	struct in6_addr sin6_addr; //IPv6 address
	u_int32_t sin6_scope_id; // Scope ID
};

struct in6_addr {
	unsigned char s6_addr[16]; // IPv6 address
};
```

> You may wonder why it bothers to define separated structure like `struct in_addr` and `struct in6_addr` to hold just one member for ip address.
>
> It used to contain more members. Actually it used to be a union not a structure. But that is proved to be unpracticed. However, for back compatibility concern, the structure format is reservered.

# get address information from accepted socket

# convert from or to printable information - modern way

Modern way of APIs supports both IPv4 and IPv6.

From binary to printable format, use `inet_ntop()`:

```c
#include <arpa/inet.h>
const char *inet_ntop(int af, const void *src,
                      char *dst, socklen_t size);
```

- af - Address family
- src - `struct in_addr` or `struct in6_addr`
- dst - buffer
- size - buffer size

From printable format to binary, use `inet_pton()`:

```c
#include <arpa/inet.h>
int inet_pton(int af, const char *src, void *dst);
```

- af - address family
- src - address string
- dst - `struct in_addr` or `struct in6_addr`

> But the numerical address string format is not that flexible than traditional `inet_aton()` and `inet_addr()`.

## A Sample Usage

