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

#include <netinet/ip.h>

void dump_ip_header(struct iphdr *ip)
{
	unsigned char *bytes = (unsigned char *)ip;

	t_icmphdr	*response_icmp = (t_icmphdr	*)ip - 1;

	printf("IP Hdr Dump:\n ");
	for (size_t i = 0; i < sizeof(struct iphdr); i+=4)
	{
		printf("%02x%02x %02x%02x ", bytes[i], bytes[i+1], bytes[i+2], bytes[i+3]);
	}


	char ip_src[INET_ADDRSTRLEN];
	char ip_dst[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(ip->saddr), ip_src, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &(ip->daddr), ip_dst, INET_ADDRSTRLEN);

	printf("\n");
	printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data\n");
	printf(" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %s %s\n",
		   ip->version, ip->ihl, ip->tos, ntohs(ip->tot_len), ntohs(ip->id),
		   ntohs(ip->frag_off) >> 13, 
		   ntohs(ip->frag_off) & 0x1FFF,
		   ip->ttl, ip->protocol, ntohs(ip->check),
		   ip_src, ip_dst
	);
	static int8_t seq_it;
	printf("ICMP: type %d, code %d, size %d, id 0x%x, seq 0x%x\n",
		response_icmp->type, response_icmp->code, ip->tot_len, ntohs(ip->id), (int32_t)seq_it 
	);
	seq_it++;
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
			break ;
			// close(p->server_sock);
			// free(p->ip_addr);
			// exit(1);
		}

		t_icmphdr		*response_icmp = (t_icmphdr *)response;
		struct iphdr	*ip = (struct iphdr *)response;
	
		uint8_t		ttl = 0;

		if (p->socket_type == TYPE_RAW)
		{
			ttl = ip->ttl;
			response_icmp = (t_icmphdr *)(response + (ip->ihl << 2));
		}

		if (response_icmp->type == ICMP_TIME_EXCEEDED)
		{
			ip = (struct iphdr *)(response_icmp + 1);
	
			char *ip_dest_str = inet_ntoa(src_addr.sin_addr);
			char *dns_lookup_str = reverse_dns_lookup(ip_dest_str);
	
			printf("%d bytes from: %s (%s): Time to live exceeded\n", DEFAULT_ERR, dns_lookup_str, ip_dest_str);
			free(dns_lookup_str);
			if (p->verbose_mode == true)
				dump_ip_header(ip);
			break;
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

		printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
			DEFAULT_READ,
			inet_ntoa(src_addr.sin_addr),
			ntohs(response_icmp->un.echo.sequence),
			ttl,
			rtt_ms
		);
		break ;
	}
}
