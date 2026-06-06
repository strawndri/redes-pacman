CC       = gcc
CFLAGS   = -Wall -Wextra -g -std=c99
BIN      = pacman
CORE_DIR = core
LIB_DIR  = lib

SRCS = main.c $(CORE_DIR)/cliente.c $(CORE_DIR)/servidor.c $(CORE_DIR)/jogo.c $(LIB_DIR)/mensagem.c $(LIB_DIR)/raw_socket.c $(LIB_DIR)/utils.c
OBJS = $(SRCS:.c=.o)
INCLUDES = -I$(CORE_DIR) -I$(LIB_DIR)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(BIN)