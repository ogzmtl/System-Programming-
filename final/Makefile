FLAGS = -c -Wall -pedantic -Wextra
SERVER = PideShop
CLIENT = HungryVeryMuch

all: $(SERVER) $(CLIENT)

$(SERVER): server.o queue.o
	gcc -o $@ $^ 

$(CLIENT): client.o queue.o
	gcc -o $@ $^

%.o : %.c
	gcc $(FLAGS) $<

clean: 
	rm -rf *.o
	rm $(SERVER)
	rm $(CLIENT)
