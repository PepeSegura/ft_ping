MAKEFLAGS    = --no-print-directory

NAME = ft_ping

CFLAGS       = -Wextra -Wall -Werror
CFLAGS      += -I inc
CFLAGS      += -O3

LIBS		= -lm

CC = cc

DEBUG        = -g3 -fsanitize=address

CPPFLAGS     = -MMD


HEADERS      = -I ./inc

FILES       =											\
				srcs/main.c								\
				srcs/init.c								\
				srcs/send.c								\
				srcs/recv.c								\
				srcs/tools.c							\
				srcs/flag_parser.c						\


OBJS = $(patsubst srcs/%.c, objs/srcs/%.o, $(FILES))

DEPS       = $(OBJS:.o=.d)

all: $(NAME)
	@- sudo setcap cap_net_raw+ep $(NAME) || true

$(NAME): $(OBJS)
	$(CC) $(DEBUG) $(OBJS) $(LIBS) $(HEADERS) -o $(NAME) && printf "Linking: $(NAME)\n"

objs/srcs/%.o: ./srcs/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(DEBUG) $(CPPFLAGS) $(CFLAGS) -o $@ -c $< $(HEADERS) && printf "Compiling: $(notdir $<)\n"

clean:
	@rm -rf objs

fclean: clean
	@rm -rf $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re