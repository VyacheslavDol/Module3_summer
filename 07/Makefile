all: A

A: msg_func.o AA
	
	gcc -o task_07_A main_A.o msg_func.o -L./library -lmyinput 
	gcc -o task_07_B main_B.o msg_func.o -L./library -lmyinput

AA:
	gcc -c main_A.c
	gcc -c main_B.c

msg_func.o:
	gcc -c msg_func.c

clean:
	rm -f *.o task_07_A task_07_B