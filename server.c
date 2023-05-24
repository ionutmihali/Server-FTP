#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define BUF (256)

void _send(int s, char *b)
{
	int n;
	// write the response to the client
	n = send(s, b, strlen(b), 0);
	if (n < 0)
	{
		perror("ERROR writing to socket");
		exit(1);
	}
}

char *_ls()
{
	DIR *d;
	struct dirent *dir;
	char *fis = (char *)malloc(sizeof(char) * BUF);
	strcpy(fis, "");
	d = opendir(".");
	if (d)
	{
		bzero(fis, BUF);
		while ((dir = readdir(d)) != NULL)
		{
			strcat(fis, dir->d_name);
			strcat(fis, "\n");
		}
	}
	closedir(d);
	return fis;
}

void _get(char *p, int s)
{
	char *buffer = (char *)malloc(sizeof(char) * BUF);
	char *fis = (char *)malloc(BUF * sizeof(char));
	fis = _ls();
	char *pp = strtok(fis, " \n\0");
	int ok = 0;
	while (pp != NULL)
	{
		if (strcmp(p, pp) == 0)
		{
			int fd = open(pp, O_RDONLY);
			int b = 0;
			int sum = 0;
			while (b = read(fd, buffer, BUF))
			{
				sum += b;
			}

			lseek(fd, 0, SEEK_SET);

			send(s, &sum, sizeof(int), 0);

			char *buffer2 = (char *)malloc(sum);
			read(fd, buffer2, sum);
			send(s, buffer2, sum, 0);

			free(buffer2);
			free(buffer);
			free(fis);

			ok = 1;

			break;
		}

		pp = strtok(NULL, "\n\0");
	}

	if (ok == 0)
	{
		_send(s, "Fisierul nu exista!");
	}
}

char *_delete(char *p)
{
	char *buffer = (char *)malloc(sizeof(char) * BUF);
	if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0 || strcmp(p, "server.c") == 0 || strcmp(p, "server") == 0 || strcmp(p, "client.c") == 0 || strcmp(p, "client") == 0)
	{
		strcpy(buffer, "Nu ai acces la stergerea acestui fisier!\n");
	}
	else
	{
		int a = unlink(p);
		if (a == 0)
		{
			strcpy(buffer, "Fisier sters cu succes!\n");
		}
		else if (a == -1)
		{
			strcpy(buffer, "Eroare la stergerea fisierului!\n");
		}
	}

	return buffer;
}

int main()
{
	int sockfd, newsockfd, portno = 5000;
	socklen_t clilen;
	char *buffer = (char *)malloc(sizeof(char) * BUF);
	int buffer_size = BUF;
	struct sockaddr_in serv_addr, cli_addr;

	// create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("ERROR opening socket");
		exit(1);
	}

	// set the address and port
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(portno);

	// bind the socket to the address and port
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR on binding");
		exit(1);
	}

	// listen for incoming connections
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	// accept a connection
	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
	if (newsockfd < 0)
	{
		perror("ERROR on accept");
		exit(1);
	}

	while (strcmp(buffer, "bye!") != 0)
	{
		// read the message from the client
		bzero(buffer, BUF);
		int n = read(newsockfd, buffer, BUF);
		char *buffer2 = (char *)malloc(BUF);
		if (n < 0)
		{
			free(buffer2);
			perror("ERROR reading from socket");
			exit(1);
		}

		char *p = strtok(buffer, " \n\0");
		if (strcmp(p, "ls") == 0)
		{
			bzero(buffer, BUF);
			buffer = _ls();
			int d = strlen(buffer);
			send(newsockfd, &d, sizeof(int), 0);

			_send(newsockfd, buffer);

			free(buffer2);
		}
		else if (strcmp(p, "get") == 0)
		{
			p = strtok(NULL, " \n\0");
			_get(p, newsockfd);
			free(buffer2);
		}
		else if (strcmp(p, "delete") == 0)
		{
			p = strtok(NULL, " \n");
			buffer = _delete(p);
			int d = strlen(buffer);
			send(newsockfd, &d, sizeof(int), 0);

			_send(newsockfd, buffer);

			free(buffer2);
		}
		else if (strcmp(p, "bye!\n") == 0)
		{
			close(newsockfd);
			free(buffer);
			free(buffer2);
		}
	}

	// close the sockets
	close(newsockfd);
	free(buffer);

	return 0;
}
