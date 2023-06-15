FLAG = -c -Wall -Wextra -pedantic

SERVER = biboServer
CLIENT = biboClient

all: $(CLIENT) $(SERVER)

%.o: %.c
	gcc $(FLAG) $<

$(SERVER) : server.o
	gcc $< -o $@

$(CLIENT) : client.o
	gcc $< -o $@

clean : 
	rm -rf *.o
	rm biboClient
	rm biboServer
