#include "ft_ping.h"

/* BONUS FLAGS */
// INIT_FLAG('f',  "flood",            NO_ARG),
// INIT_FLAG('l',  "preload",          NEED_ARG),
// INIT_FLAG('n',  "numeric",          NO_ARG),
// INIT_FLAG('w',  "timeout",          NEED_ARG),
// INIT_FLAG('W',  "linger",           NEED_ARG),
// INIT_FLAG('p',  "pattern",          NEED_ARG),
// INIT_FLAG('r',  "ignore-routing",   NO_ARG),
// INIT_FLAG('s',  "size",             NEED_ARG),
// INIT_FLAG('T',  "tos",              NEED_ARG),
// INIT_FLAG(0,    "ttl",              NEED_ARG),
// INIT_FLAG(0,    "ip-timestamp",     NEED_ARG),

// int main(int argc, char **argv)
// {
//     t_flag flags[] = {
//         /* Mandatory */
//         INIT_FLAG('v',  "verbose",          NO_ARG),
//         INIT_FLAG('?',  "help",             NO_ARG),
//         /* Bonus */
//     };

//     t_flag_parser flag_parser = parser_init(flags, FLAGS_COUNT(flags), argc, argv);

// 	parse(&flag_parser);

// 	print_parsed_flags(&flag_parser);

// 	cleanup_parser(&flag_parser);
//     return (0);
// }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

char	*resolve_hostname(const char *hostname)
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
		return (NULL);
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

	printf("%s (%s):\n", hostname, ip_str);

	freeaddrinfo(result);
	return (strdup(ip_str));
}

int main(int argc, char **argv)
{
	if (argc != 2)
		return (1);
	char *ip_address_str = resolve_hostname(argv[1]);
	if (ip_address_str)
		free(ip_address_str);
	return (0);
}
