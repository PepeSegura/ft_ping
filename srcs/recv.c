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

void dump_ip_header(struct iphdr *ip) {
    unsigned char *bytes = (unsigned char *)ip;

	printf("IP Hdr Dump:\n ");
    // Print hex dump (like "4500 0054 ...")
    for (size_t i=0; i<sizeof(struct iphdr); i+=4)
	{
        printf("%02x%02x %02x%02x ", bytes[i], bytes[i+1], bytes[i+2], bytes[i+3]);
    }

    // Human-readable format
	printf("\n");
    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data\n");
    printf(" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x\n",
           ip->version, ip->ihl, ip->tos, 
           ntohs(ip->tot_len), ntohs(ip->id),
           ntohs(ip->frag_off) >> 13, 
           ntohs(ip->frag_off) & 0x1FFF,
           ip->ttl, ip->protocol, ntohs(ip->check));
	printf("ICMP: type 8, code 0, size 64, id 0x5940, seq 0x0000\n");
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

		if (response_icmp->type == ICMP_TIME_EXCEEDED)
		{
			struct iphdr	*original_ip = (struct iphdr *)(response_icmp + 1); // Pointer to original IP header
	
			char *ip_dest_str = inet_ntoa(src_addr.sin_addr);
			char *dns_lookup_str = reverse_dns_lookup(ip_dest_str);

			printf("92 bytes from: %s (%s): Time to live exceeded\n", dns_lookup_str, ip_dest_str);
			free(dns_lookup_str);
			if (p->verbose_mode == true)
			{
				dump_ip_header(original_ip);
			}
			// printf("  Original destination: %s\n", inet_ntoa(*(struct in_addr *)&original_ip->daddr));
			// printf("  Original ICMP type: %d\n", original_icmp->type);
			// printf("  Original ICMP code: %d\n", original_icmp->code);
			// printf("  Original ICMP ID: %d\n", ntohs(original_icmp->un.echo.id));
			// printf("  Original ICMP seq: %d\n", ntohs(original_icmp->un.echo.sequence));

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
			ret_recv,
			inet_ntoa(src_addr.sin_addr),
			ntohs(response_icmp->un.echo.sequence),
			ttl,
			rtt_ms
		);
		break ;
	}
}
