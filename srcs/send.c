#include "ft_ping.h"

// typedef struct s_payload{
// 	uint16_t sequence;
// 	struct timeval timestamp;
// 	char data[48];
// } t_payload;

// typedef struct s_packet {
// 	struct icmphdr hdr;
// 	t_payload payload;
// }   t_packet;


void send_packet(t_ping *p)
{
	t_packet	pkt;
	size_t		curr_sequence = p->sequence++;
	
	memset(&pkt, 0, sizeof(t_packet));
	
	// printf("SETTING [%s] PACKET...\n", (p->socket_type == TYPE_RAW) ? "RAW" : "DGRAM");
	pkt.hdr.type = ICMP_ECHO; // 8 = echo request - 0 = echo reply
	pkt.hdr.code = 0;
	pkt.hdr.un.echo.id = htons(getpid());
	pkt.hdr.un.echo.sequence = htons(curr_sequence);

	gettimeofday(&pkt.payload.timestamp, NULL);
	memset(pkt.payload.data, 'A', sizeof(pkt.payload.data));

	pkt.hdr.checksum = 0;
	pkt.hdr.checksum = htons(checksum((void *)&pkt, sizeof(pkt)));

	struct sockaddr_in dest_addr = {
		.sin_family		 	= AF_INET,
		.sin_addr.s_addr	= inet_addr(p->ip_addr),
	};

	int ret_send = sendto(
		p->server_sock, &pkt, sizeof(pkt), 0,
		(struct sockaddr *)&dest_addr, sizeof(dest_addr)
	);

	if (ret_send < 0)
	{
		perror("sendto");
		close(p->server_sock);
		free(p->ip_addr);
		exit(1);
	}
	// printf("Sent ICMP Echo Request to %s (seq=%ld)\n", p->hostname, curr_sequence);
}
