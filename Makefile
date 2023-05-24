all: serv cli

serv: server
	gcc server.c -o server

cli: client
	gcc client.c -o client