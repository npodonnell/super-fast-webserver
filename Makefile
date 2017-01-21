all:
	gcc -o event-server.exe main.c server.c client.c ep.c -Wall -O3 -s
