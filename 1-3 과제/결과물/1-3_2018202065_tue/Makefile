OBJS = proxy_cache.c
CC = gcc
EXEC = proxy_cache

all: $(OBJS)
	$(CC) -o $(EXEC) $^ -lcrypto

clean:
	rm -rf $(EXEC)
