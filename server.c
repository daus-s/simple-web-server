

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_REQ_BYTES 128

void *client_handler(void *arg)
{
    int sockfd;

    sockfd = *(int *)arg;

    char* buf;
    buf = malloc(MAX_REQ_BYTES);

    int n = read(sockfd, buf, MAX_REQ_BYTES);
    if (n < 0)
    {
        perror("Could not read from socket");
    }
    buf[n] = '\0';
    printf("%s\n", buf);

    strtok(buf, " ");
    char* filename = strtok(NULL, " ");
    if (strcmp(filename, "/")==0)
    {
        strcpy(filename, "index.html");
    }

    int fd = open(filename, O_RDONLY);
    printf("name:%s\nid:%d\n", filename, fd);

    char* w;
    w =  malloc(256);


    write(sockfd, "HTTP/1.1 200 OK\n\n", 17);
    while (read(fd,w,256)>0)
    {
        printf("%s\n", w);
        write(sockfd, w, 256);
    }
    write(sockfd, "\n\n\0", 3);
    //snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));


    close(sockfd);
    close(fd);
    free(buf);
    free(w);

}

int main(int argc, char **argv)
{

   const int backlog = 5;

   struct  sockaddr_in  server_addr;
   struct  sockaddr_in  client_addr;
   pthread_t tid;

   int sockfd, client_sockfd;
   int serverlen, clientlen;


   if (argc != 3)
   {
       printf("Usage: %s <ip-address> <port> \n", argv[0]);
       return -1;
   }

   /* Create the socket */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd == -1)
   {
       perror("Could not create socket");
       return -1;
   }

   /* Name the socket */
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = inet_addr(argv[1]);
   server_addr.sin_port = htons(atoi(argv[2]));

   /* bind to server socket */
   if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
   {
       perror("Could not bind to socket");
       close(sockfd);
       return -1;
   }

   /* wait for client to connect */
   listen(sockfd, backlog);

   while (true)
   {

       /* Accept a connection */
       clientlen = sizeof(client_addr);
       client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &clientlen);
       if (client_sockfd == -1)
       {
           perror("Unable to accept client connection request");
           continue;
       }

       if (pthread_create(&tid, NULL, client_handler, (void *)&client_sockfd) < 0)
       {
           perror("Unable to create client thread");
           break;
       }

   }

   close(sockfd);

   return 0;

}
