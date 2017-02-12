all: release


test: release
	valgrind --tool=memcheck --leak-check=yes ./event-server.exe

release:
	gcc -o event-server.exe main.c server.c client.c ep.c -Wall -O3 -s
