all:
	gcc -o event-server.exe main.c server.c client.c -Wall -O3 -s
