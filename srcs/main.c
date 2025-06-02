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
	printf("--- %s ping statistics ---\n", p->hostname);
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
		p->send_count, p->read_count,
		100 - ((p->read_count * 100) / p->send_count)
	);
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
		p->rtt_s.min, p->rtt_s.mean, p->rtt_s.max, p->rtt_s.stddev
	);
}

int main(int argc, char **argv)
{
	finish = false;

	signal(SIGINT, sig_handler);

	if (argc < 2)
	{
		dprintf(2, "Usage: %s <destination IP>\n", argv[0]);
		return (1);
	}

	t_flag flags[] = {
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

	t_flag_parser flag_parser = parser_init(flags, FLAGS_COUNT(flags), argc, argv);

	parse(&flag_parser);
	print_parsed_flags(&flag_parser);

	int	pos_flag;

	if ((pos_flag = check_flag(&flag_parser, '?', "help")) != -1)
	{
		printf("%s\n", HELP);
		cleanup_parser(&flag_parser);
		exit(EXIT_SUCCESS);
	}

	cleanup_parser(&flag_parser);
	
	t_ping p;

	memset(&p, 0, sizeof(t_ping));
	init_t_ping(&p, argv);

	while (finish == false)
	{
		send_packet(&p);
		recv_packet(&p);
		usleep(1.0 * 1000 * 1000);
	}
	print_stats(&p);
	close(p.server_sock);
	free(p.ip_addr);
	return (0);
}
