#include "ft_ping.h"

static char	*resolve_hostname(const char *hostname)
{
	struct addrinfo in_info, *result;
	char ip_str[INET6_ADDRSTRLEN];

	memset(&in_info, 0, sizeof(in_info));
	in_info.ai_family = AF_UNSPEC;
	in_info.ai_socktype = SOCK_RAW;

	int ret;
	if ((ret = getaddrinfo(hostname, NULL, &in_info, &result)) != 0)
	{
		dprintf(2, "getaddrinfo: %s\n", gai_strerror(ret));
        exit(EXIT_FAILURE);
	}

	void	*addr;

	if (result->ai_family == AF_INET)
	{
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)result->ai_addr;
		addr = &(ipv4->sin_addr);
	}
	else
	{
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)result->ai_addr;
		addr = &(ipv6->sin6_addr);
	}
	inet_ntop(result->ai_family, addr, ip_str, sizeof(ip_str));

	// printf("%s (%s):\n", hostname, ip_str);

	freeaddrinfo(result);
	return (strdup(ip_str));
}

# define INVALID_FD -1

void create_server_socket(t_ping *ping)
{
    int server_fd = INVALID_FD;

    server_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (server_fd != INVALID_FD)
    {
        ping->socket_type = TYPE_RAW;
        goto set_socket_flags;
    }
    server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (server_fd != INVALID_FD)
    {
        ping->socket_type = TYPE_DGRAM;
        goto set_socket_flags;
    }
    perror("socket");
    exit(EXIT_FAILURE);

    set_socket_flags:
        int flags = fcntl(server_fd, F_GETFL, 0);

        fcntl(server_fd, F_SETFL, flags & ~O_NONBLOCK); // Disable non-blocking

        struct timeval tv = {.tv_sec = 2, .tv_usec = 0};
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); // set 2 secs timeout
    ping->server_sock = server_fd;
}

void    init_t_ping(t_ping *ping, char **argv)
{
    ping->hostname = argv[1];
    ping->ip_addr = resolve_hostname(argv[1]);

    create_server_socket(ping);

    printf("--------------------------\n");
    printf("Hostname:  %s\n", ping->hostname);
    printf("IP_Dest:   %s\n", ping->ip_addr);
    printf("Sock_type: [%s]\n", (ping->socket_type == TYPE_RAW) ? "RAW" : "DGRAM");
    printf("Sock_fd:   %d\n", ping->server_sock);
    printf("--------------------------\n");
}
