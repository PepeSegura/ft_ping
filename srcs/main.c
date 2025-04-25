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

int main(void)
{

}


int	create_server_socket(int port)
{
	int	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	int optval = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	{
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}
	//Used to make the socket non-blocking
	if (fcntl(server_socket, F_SETFL, O_NONBLOCK))
	{
		perror("fcntl failed");
		exit(EXIT_FAILURE);
	}
	if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_socket, MAX_CLIENTS) < 0)
	{
		perror("listen failed");
		exit(EXIT_FAILURE);
	}
	return (server_socket);
}
