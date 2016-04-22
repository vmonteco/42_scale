NAME =		42_scale
CC = 		gcc
CFLAGS = 	-Wall -Wextra -Werror -g -O2
LIBS = 		-lglfw3 -framework OpenGL -lm -lGLEW
LIBS_DIR =	-L ~/.brew/lib
INCS = 		-I inc/ -I inc/nuklear/ -I ~/.brew/include
SRCS = 		$(wildcard srcs/*.c)
OBJS =		$(SRCS:%.c=%.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -o $@ -c $^

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(LIBS_DIR) $(LIBS) $(OBJS) -o $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean fclean re
