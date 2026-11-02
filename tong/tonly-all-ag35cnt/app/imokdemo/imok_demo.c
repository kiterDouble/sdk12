#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>


#define UDP_PORT       30032
#define MAX_LINE       220 

int main(int argC, char* arg[])

{
    struct sockaddr_in server;
    int sockfd, len = 0;   
    size_t server_len = sizeof(struct sockaddr_in);    

    char buf[MAX_LINE];

    /* setup a socket，attention: must be SOCK_DGRAM */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
   
        perror("socket");
        exit(1);
    }
 
    /*complete the struct: sockaddr_in*/
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(UDP_PORT);
    server.sin_addr.s_addr = inet_addr(arg[1]);

  //  while(1) {
        /* send the string to server*/
        if(sendto(sockfd, arg[2],  strlen(arg[2]), 0, (struct sockaddr *)&server, server_len)<0) //sizeof
        {
            printf("sendto error\n");
            exit(1);
        }

        memset(buf, 0, sizeof(buf));

        /* recieve the string from server*/       
        len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&server, &server_len);
        if(len <0)
        {
           printf("recvfrom error\n");
           exit(1);
        }
        printf("%s\n", buf);

  //  }
	close(sockfd);
    return 0;
}
