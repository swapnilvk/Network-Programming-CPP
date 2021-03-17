// print the IP addresses for whatever host you specify on the command line:

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int ingc, char* argv[])
{   
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if(ingc != 2)
    {
        fprintf(stderr, "usage: show ip hostname");
        return 1;
    }
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(status = getaddrinfo(argv[1],NULL,&hints,&res)!=0)
    {
        fprintf(stderr, "getaddrininfo: %s\n", gai_strerror(status));
        return 2;
    }
    printf("IP addresses for %s:\n\n", argv[1]);
    for(p=res; p!=NULL; p=p->ai_next)
    {
        void *addr;
        char *ipver;

        if(p->ai_family == AF_INET) //IPV4
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPV4";

        }
        else //IPV6
        {
            struct sockaddr_in6 *ipv4 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv4->sin6_addr);
            ipver = "IPV6";
        }

        //Convert IP to string and print 
        inet_ntop(p->ai_family,addr,ipstr, sizeof (ipstr));
        printf("  %s: %s\n", ipver, ipstr);
    }
    freeaddrinfo(res); // free the linked list

    return 0;
}

/* Output:

swapnil@swapnil-pc /media/swapnil/Network-Programming-CPP $ ./Show_IP www.google.com
IP addresses for www.google.com:
IPV4: 142.250.77.36
IPV6: 2404:6800:4009:81e::2004 

*/
