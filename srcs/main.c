#include "ft_ping.h"

bool finish;

void sig_handler(int signum)
{
	if (signum == SIGINT)
	{
		finish = true;
	}
}

void print_stats(t_ping *p)
{
	int	packet_loss = 100 - ((p->read_count * 100) / p->send_count);

	printf("--- %s ping statistics ---\n", p->hostname);
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
		p->send_count, p->read_count, packet_loss
	);
	if (packet_loss == 100)
		return ;
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
		p->rtt_s.min, p->rtt_s.mean, p->rtt_s.max, p->rtt_s.stddev
	);
}

int main(int argc, char **argv)
{
	finish = false;

	signal(SIGINT, sig_handler);

	t_flag available_flags[] = {
		/* Mandatory */
		INIT_FLAG('v',	"verbose",	NO_ARG),
		INIT_FLAG('?',	"help",		NO_ARG),
		/* Bonus */
		INIT_FLAG('c',	"count",	NEED_ARG),	// number of packets to send
		INIT_FLAG('i',	"interval",	NEED_ARG),	// interval between sendind packets (in seconds)
		INIT_FLAG(0,	"ttl",		NEED_ARG),	// secify N as time-to-live
		INIT_FLAG('w',	"timeout",	NEED_ARG),	// stop after N seconds
		INIT_FLAG('q',	"quiet",	NO_ARG	),	// quiet output
	};

	t_flag_parser flags = parser_init(available_flags, FLAGS_COUNT(available_flags), argc, argv);

	parse(&flags);
	// print_parsed_flags(&flags);

	t_ping p;

	for (size_t i = 0; i < flags.extra_args_count; i++)
	{
		memset(&p, 0, sizeof(t_ping));
		init_t_ping(&p, flags.extra_args[i], &flags);

		while (finish == false && p.send_count < p.send_limit)
		{
			send_packet(&p);
			recv_packet(&p);
			usleep(p.send_interval * 1000 * 1000);
		}

		print_stats(&p);
		close(p.server_sock);
		free(p.ip_addr);
	}
	cleanup_parser(&flags);

	return (0);
}
