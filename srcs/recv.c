#include "ft_ping.h"

void recv_packet(t_ping *p)
{
	while (1)
	{
		byte response[1024] = {0};

		struct sockaddr_in src_addr;
		socklen_t src_len = sizeof(src_addr);

		int ret_recv = recvfrom(
			p->server_sock, response, sizeof(response), 0,
			(struct sockaddr *)&src_addr, &src_len
		);
		if (ret_recv < 0)
		{
			if (errno != EINTR)
				perror("recvfrom ret < 0");
			close(p->server_sock);
			free(p->ip_addr);
			exit(1);
		}

		t_icmphdr	*response_icmp = (t_icmphdr *)response;
		uint8_t		ttl = 0;

		if (p->socket_type == TYPE_RAW)
		{
			struct iphdr *ip = (struct iphdr *)response;
			ttl = ip->ttl;
			response_icmp = (t_icmphdr *)(response + (ip->ihl * 4));
		}

		if (response_icmp->type != ICMP_ECHOREPLY) continue ;

		if (p->socket_type == TYPE_RAW && response_icmp->un.echo.id != htons(getpid()))
		{
			dprintf(2, "Invalid ID\n");
			continue ;
		}

		t_payload *payload = (t_payload *)(response_icmp + 1);

		double rtt_ms = time_diff(&payload->timestamp);

		printf("64 bytes from %s: icmp_seq=%d ttl=%d time=%.2fms\n",
			inet_ntoa(src_addr.sin_addr),
			ntohs(response_icmp->un.echo.sequence),
			ttl,
			rtt_ms
		);
		break ;
	}
	p->read_count++;
}
