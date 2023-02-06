OBJS1 = server.c
OBJS2 = client.c
CC = gcc
EXEC1= server
EXEC2=client
all: first second
first: $(OBJS1)
	$(CC) -o $(EXEC1) $^ -lcrypto
second: $(OBJS2)
	$(CC) -o $(EXEC2) $^

clean:
	rm -rf $(EXEC)
