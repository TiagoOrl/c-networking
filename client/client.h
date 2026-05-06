#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080



void connect()
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;

    char* hello = "Hello From Client\n";
    char buffer[1024] = {0};

    if ()
    {
        
    }
}
