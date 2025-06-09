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
	int	packet_loss = 100;
	if (p->send_count)
		packet_loss = 100 - ((p->read_count * 100) / p->send_count);
	

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

void	check_help_usage(t_flag_parser *flags)
{
	int	pos_flag;

	if (flags->argc < 2 || flags->extra_args_count < 1)
	{
		dprintf(2, "%s: missing host operand\n", flags->argv[0]);
		dprintf(2, "Try '%s --help' or '%s --usage' for more information.\n", flags->argv[0], flags->argv[0]);
		exit(EXIT_FAILURE);
	} if ((pos_flag = check_flag(flags, '?', "help"))	!= -1) {
		printf("%s\n", HELP);
		cleanup_parser(flags);
		exit(EXIT_SUCCESS);
	} if ((pos_flag = check_flag(flags, 0, "usage"))	!= -1) {
		printf("%s\n", USAGE);
		cleanup_parser(flags);
		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char **argv)
{

	finish = false;

	signal(SIGINT, sig_handler);

	t_flag available_flags[] = {
		/* Mandatory */
		INIT_FLAG('v',	"verbose",	NO_ARG),
		INIT_FLAG('?',	"help",		NO_ARG),
		INIT_FLAG(0,	"usage",	NO_ARG),
		/* Bonus */
		INIT_FLAG('c',	"count",	NEED_ARG),	// number of packets to send
		INIT_FLAG('i',	"interval",	NEED_ARG),	// interval between sendind packets (in seconds)
		INIT_FLAG(0,	"ttl",		NEED_ARG),	// secify N as time-to-live
		INIT_FLAG('w',	"timeout",	NEED_ARG),	// stop after N seconds
		INIT_FLAG('q',	"quiet",	NO_ARG	),	// quiet output
	};

	t_flag_parser flags = parser_init(available_flags, FLAGS_COUNT(available_flags), argc, argv);

	parse(&flags);
	check_help_usage(&flags);
	// print_parsed_flags(&flags);

	t_ping p;

	double ping_start_ms = get_time_in_ms();

	for (size_t i = 0; i < flags.extra_args_count; i++)
	{
		memset(&p, 0, sizeof(t_ping));
		p.ping_start = ping_start_ms;
		init_t_ping(&p, flags.extra_args[i], &flags);

		while (finish == false && p.send_count < p.send_limit && get_time_in_ms() < p.time_end)
		{
			send_packet(&p);
			recv_packet(&p);
			if (finish == false && p.send_count < p.send_limit && get_time_in_ms() < p.time_end)
				usleep(p.send_interval * 1000 * 1000);
		}

		print_stats(&p);
		close(p.server_sock);
		free(p.ip_addr);
	}
	cleanup_parser(&flags);
	return (0);
}
