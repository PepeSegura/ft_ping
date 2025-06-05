#include "ft_ping.h"

static char	*resolve_hostname(const char *hostname)
{
	struct addrinfo in_info, *result;
	char ip_str[INET6_ADDRSTRLEN];

	memset(&in_info, 0, sizeof(in_info));
	in_info.ai_family = AF_INET;
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
    int socket_flags = 0;

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
        socket_flags = fcntl(server_fd, F_GETFL, 0);

        fcntl(server_fd, F_SETFL, socket_flags & ~O_NONBLOCK); // Disable non-blocking

        struct timeval tv = {.tv_sec = 2, .tv_usec = 0};
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); // set 2 secs timeout
    ping->server_sock = server_fd;
}

void    check_flags(t_ping *ping)
{
	int	pos_flag;

	if ((pos_flag = check_flag(ping->flags, '?', "help")) != -1) {
    	printf("%s\n", HELP);
		cleanup_parser(ping->flags);
		exit(EXIT_SUCCESS);
    } if ((pos_flag = check_flag(ping->flags, 'v', "verbose"))     != -1) {
        ping->verbose_mode = true;
    } if ((pos_flag = check_flag(ping->flags, 'c', "count"))       != -1) {
        ping->send_limit = atoi(ping->flags->flags[pos_flag].args[0]);
    } if ((pos_flag = check_flag(ping->flags, 'i', "interval"))    != -1) {
        ping->send_interval = atof(ping->flags->flags[pos_flag].args[0]);
    } if ((pos_flag = check_flag(ping->flags, 0, "ttl"))           != -1) {
        ping->custom_ttl = atoi(ping->flags->flags[pos_flag].args[0]);
    } if ((pos_flag = check_flag(ping->flags, 'w', "timeout"))     != -1) {
        ping->total_runtime = atoi(ping->flags->flags[pos_flag].args[0]);
    } if ((pos_flag = check_flag(ping->flags, 'q', "quiet"))       != -1) {
        ping->quiet_mode = true;
    }

    if (ping->flags->extra_args_count < 1)
    {
		dprintf(2, "Usage: %s <destination IP>\n", ping->flags->argv[0]);
        cleanup_parser(ping->flags);
		exit(EXIT_FAILURE);
    }
}

void    init_t_ping(t_ping *ping, char *host, t_flag_parser *flags)
{
    ping->custom_ttl = -1;
    ping->send_limit = INT32_MAX;
    ping->total_runtime = INT32_MAX;
    ping->send_interval = 1.0;

    ping->flags = flags;
    check_flags(ping);

    ping->hostname = host;
    ping->ip_addr = resolve_hostname(host);

    create_server_socket(ping);

    gettimeofday(&ping->time_start, NULL);
    ping->rtt_s.max = (double)INT64_MIN;
    ping->rtt_s.min = (double)INT64_MAX;

    if (ping->verbose_mode == false) {
        printf("PING %s (%s): 56 data bytes\n", ping->hostname, ping->ip_addr);
    } else {
        size_t pid = getpid();
        printf("PING %s (%s): 56 data bytes, id %p = %ld\n", ping->hostname, ping->ip_addr, (void *)pid, pid);
    }
}
