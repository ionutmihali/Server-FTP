#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF (256)

int main()
{
    int sockfd, n, portno = 5000;
    struct sockaddr_in serv_addr;
    char buffer[BUF];

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

    // connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        exit(1);
    }

    printf("> ");
    bzero(buffer, BUF);

    while (fgets(buffer, sizeof(buffer), stdin))
    {
        n = send(sockfd, buffer, BUF, 0);
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }

        if (strcmp(buffer, "bye!\n") == 0)
        {
            break;
        }

        // read the response from the server
        bzero(buffer, BUF);

        int fin;
        n = recv(sockfd, &fin, sizeof(int), 0);
        if (n < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }

        // n = send(sockfd, "ACK", strlen("ACK"), 0);

        char *buffer2 = (char *)malloc(fin);
        n = recv(sockfd, buffer2, fin, 0);
        buffer2[fin] = '\0';

        // print the response
        printf("%s", buffer2);

        free(buffer2);
        printf("\n> ");
    }

    // close the socket
    close(sockfd);
    return 0;
}
