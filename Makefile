NAME = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
SRC = main.c parse.c simulation.c time.c heap.c heap_push.c heap_pop.c heap_utils.c dongles.c
OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c codexion.h heap_internal.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
