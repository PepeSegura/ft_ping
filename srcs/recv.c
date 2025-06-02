#include "ft_ping.h"

void mean_and_stddev(t_ping *p, double new_value)
{
	t_rtt_stats *r = (t_rtt_stats *)&p->rtt_s;

	r->total_count = p->read_count;

	double delta = new_value - r->mean;
	r->mean += delta / r->total_count;

	double delta2 = new_value - r->mean;

	r->m2 += delta * delta2;

	double variance = 0.0;
	if (r->total_count > 1)
		variance = (r->m2 / (r->total_count - 1));
	r->stddev = sqrt(variance);
}

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

		if (response_icmp->type == ICMP_TIME_EXCEEDED) {
			// Print debug info when verbose mode is on
			break ;
		}
		if (response_icmp->type != ICMP_ECHOREPLY) continue ;

		if (p->socket_type == TYPE_RAW && response_icmp->un.echo.id != htons(getpid()))
		{
			dprintf(2, "Invalid ID\n");
			continue ;
		}

		p->read_count++;

		t_payload *payload = (t_payload *)(response_icmp + 1);

		double rtt_ms = time_diff(&payload->timestamp);

		if (rtt_ms > p->rtt_s.max) p->rtt_s.max = rtt_ms;
		if (rtt_ms < p->rtt_s.min) p->rtt_s.min = rtt_ms;
		
		mean_and_stddev(p, rtt_ms);

		if (p->quiet_mode == true) break;

		printf("64 bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
			inet_ntoa(src_addr.sin_addr),
			ntohs(response_icmp->un.echo.sequence),
			ttl,
			rtt_ms
		);
		break ;
	}
}
