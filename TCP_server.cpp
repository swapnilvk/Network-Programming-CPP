#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<errno.h>

int main()
{
    int sock, cli;
    struct sockaddr_in server, client;
    unsigned int len =0;
    char msg[] = {"\n Welcome to socket "};
    int sent = 0;

    if((sock=socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
        perror("Socket could not exist\n");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(10000);
    server.sin_addr.s_addr = INADDR_ANY;    
    bzero(&server.sin_zero,8);

    len = sizeof(struct sockaddr_in);

    if(bind(sock,(struct sockaddr *)&server,len) == -1)
    {
        perror("Error in bind\n");
        exit(1);
    }
    if(listen(sock, 5) == -1)
    {
        perror("Error in listen\n");
        exit(1);
    }

    while(1)
    {
        if(cli= accept(sock,(struct sockaddr*)&client,&len)==-1)
        {
            perror("Error in accept\n");
            exit(1);
        }
        if(sent = send(cli, msg, strlen(msg),0)== -1)
        {
            perror("Error in send :%d \n");
             if( sent != 0 )
            {
                char buffer[ 256 ];
                strerror_r( errno, buffer, 256 ); // get string message from errno, XSI-compliant version
                printf("Error %s", buffer);
                // or
                char * errorMsg = strerror_r( errno, buffer, 256 ); // GNU-specific version, Linux default
                printf("Error %s", errorMsg); //return value has to be used since buffer might not be modified
                // ...
            }
            exit(1);
        }
        printf("Sent %d bytes to client: %s", sent,inet_ntoa(client.sin_addr));

       close(cli);
    }

    return 0;   
}

