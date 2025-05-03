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


typedef struct icmphdr t_icmphdr;

typedef unsigned char byte;

uint16_t checksum(void *buffer, size_t len)
{
	uint8_t		*bytes = (uint8_t *)buffer;
	uint32_t	sum = 0;

	size_t	i = 0;
	while (i + 1 < len)
	{
		uint16_t two_bytes = (bytes[i] << 8 | bytes[i + 1]);
		sum += two_bytes;
		i++;
	}
	if (len % 2 == 1)
	{
		uint16_t two_bytes = (bytes[len - 1] << 8 | 0);
		sum += two_bytes;
	}
	// printf("sum: [%d]\n", sum);
	// printf("sum>>16: [%d]\n", sum >> 16);

	while (sum >> 16 != 0)
	{
		// printf("loop-sum>>16: [%d]\n", sum >> 16);

		// printf("bef-sum: [%d]\n", sum);
		sum = (sum & 0xFFFF) + (sum >> 16);
		// printf("aft-sum: [%d]\n", sum);
	}
	// printf("afterloop-sum>>16: [%d]\n", sum >> 16);
	sum = (uint16_t)(~sum);
	printf("FINAL-sum: [%d]\n", sum);

	return (sum);
}

int create_server_socket()
{
	int server_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (server_fd < 0)
	{
		perror("socket");
		return (1);
	}
	int flags = fcntl(server_fd, F_GETFL, 0);
	fcntl(server_fd, F_SETFL, flags & ~O_NONBLOCK); // Disable non-blocking
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));	
	return (server_fd);
}

int main(int argc, char **argv)
{
	(void)argc, (void)argv;
	if (argc != 2)
	{
		dprintf(2, "Usage: %s <destination IP>\n", argv[0]);
		return (1);
	}

	byte packet[64] = {0};

	t_icmphdr *icmp = (t_icmphdr *)packet;
	printf("SIZEE: %ld\n", sizeof(t_icmphdr));
	printf("SIZEE2: %ld\n", sizeof(*icmp));

	icmp->type = ICMP_ECHO; // 8 = echo request - 0 = echo reply
	icmp->code = 0;
	icmp->un.echo.id = htons(getpid());
	icmp->un.echo.sequence = htons(1);
	memset(packet + sizeof(t_icmphdr), 'A', sizeof(packet)- sizeof(t_icmphdr));

	icmp->checksum = checksum(packet, sizeof(packet));

	int server_fd = create_server_socket();

	char *ip_addr_dest = resolve_hostname(argv[1]);
	if (ip_addr_dest == NULL)
	{
		close(server_fd);
		return (1);
	}

    struct sockaddr_in dest_addr = {
    	.sin_family = AF_INET,
    	.sin_addr.s_addr = inet_addr(ip_addr_dest),
	};
    // memset(&dest_addr, 0, sizeof(dest_addr));

	int ret_send = sendto(server_fd, packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (ret_send < 0)
	{
		perror("sendto");
		close(server_fd);
		free(ip_addr_dest);
		return (1);
	}

	printf("Sent ICMP Echo Request to %s\n", argv[1]);

	byte response[1024];

	struct sockaddr_in src_addr;
	socklen_t src_len = sizeof(src_addr);

	while (1)
	{
		int ret_recv = recvfrom(server_fd, response, sizeof(response), 0, (struct sockaddr *)&src_addr, &src_len);
		if (ret_recv < 0)
		{
			perror("recvfrom");
			close(server_fd);
			free(ip_addr_dest);
			return (1);
		}

		struct iphdr *ip = (struct iphdr *)response;
		t_icmphdr *response_icmp = (t_icmphdr *)(response + (ip->ihl * 4));
		printf("type: %d\n", response_icmp->type);
		if (response_icmp->type != ICMP_ECHOREPLY) continue;
		if (ip->saddr != dest_addr.sin_addr.s_addr) continue;
		if (response_icmp->un.echo.id != htons(getpid())) continue;

		if (response_icmp->type == ICMP_ECHOREPLY)
		{
			printf(
				"Received ICMP Echo Reply from %s (ID=%d, Seq=%d)\n",
				inet_ntoa(src_addr.sin_addr),
				ntohs(response_icmp->un.echo.id),
				ntohs(response_icmp->un.echo.sequence)
			);
		}
		break;
	}
	close(server_fd);
	free(ip_addr_dest);
	return (0);
}

	// struct icmp_header *icmp_header = (struct icmp_header);
	
	// (void)argc, (void)argv;
	// if (argc != 2)
	// 	return (1);
	// char *ip_address_str = resolve_hostname(argv[1]);
	// if (ip_address_str)
	// 	free(ip_address_str);