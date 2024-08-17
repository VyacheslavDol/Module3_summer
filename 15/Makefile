all: S C
	
S: O
	gcc main_server.o input_char.o mymaths.o -o server

C: O
	gcc main_client.o input_char.o mymaths.o -o client
	
O:
	gcc -c main_server.c main_client.c input_char.c mymaths.c


clean:
	rm -f *.o client server

cleanob:
	rm -f *.o