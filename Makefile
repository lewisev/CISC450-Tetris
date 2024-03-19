client: client.c
	gcc -o client client.c -lncurses
server: tetris_server.c
	gcc -o tetris_server tetris_server.c -lncurses
clean:
	rm client tetris_server
tetris: client server