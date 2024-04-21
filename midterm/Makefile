FLAG = -c -Wall -Wextra -pedantic

SERVER = neHosServer
CLIENT = neHosClient

all : $(SERVER) $(CLIENT)

%.o  : %.c
	gcc $(FLAG) $^

$(SERVER) : server.o common.o
	gcc -o $@ $^

$(CLIENT): client.o common.o
	gcc -o $@ $^

## do we need to remove our created folder

clean: 
	rm -rf *.o
	rm neHosServer
	rm neHosClient
	rm -rf *.txt


