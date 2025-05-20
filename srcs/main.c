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
	printf("\n--- %s ping statistics ---\n", p->hostname);
	printf("%d packets transmitted, %d received, %d%% packet loss, time %.0fms\n",
		p->send_count, p->read_count,
		100 - ((p->read_count * 100) / p->send_count),
		time_diff(&p->time_start)
	);
	printf("rtt min/avg/max/mdev = 5.542/5.567/5.592/0.025 ms\n");
}

int main(int argc, char **argv)
{
	finish = false;

	signal(SIGINT, sig_handler);

	(void)argc, (void)argv;
	if (argc != 2)
	{
		dprintf(2, "Usage: %s <destination IP>\n", argv[0]);
		return (1);
	}
	
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
