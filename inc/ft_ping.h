#ifndef FT_PING_H
# define FT_PING_H

# include "flag_parser.h"

# include <math.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <errno.h>

# include <sys/socket.h>
# include <sys/time.h>
# include <sys/types.h>

# include <arpa/inet.h>

# include <netdb.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>

# include <fcntl.h>
# include <poll.h>

# include "help.h"

enum sock_type {
    TYPE_RAW,
    TYPE_DGRAM,
};

typedef struct s_rtt_stats {
	int		total_count;

	double	min;
	double	max;
	double	m2;

	double	mean;
	double	stddev;
} t_rtt_stats;

typedef struct s_ping {
	char		*hostname;
	char		*ip_addr;
	int			socket_type;
	int			server_sock;

    int     	sequence;
	int			send_count;
	int			read_count;

	t_rtt_stats	rtt_s;

	struct timeval time_start;
	struct timeval time_last;
}	t_ping;

typedef struct s_payload{
	uint16_t		sequence;
	struct timeval	timestamp;
	char			data[64];
} t_payload;

typedef struct s_packet {
	struct icmphdr hdr;
	t_payload payload;
}   t_packet;

typedef struct icmphdr t_icmphdr;
typedef uint8_t byte;

/* init.c */
void        init_t_ping(t_ping *ping, char **argv);

/* tools.c */
uint16_t    checksum(void *buffer, size_t len);
double      time_diff(struct timeval *prev);

/* send.c */
void send_packet(t_ping *ping);
void recv_packet(t_ping *ping);

#endif
