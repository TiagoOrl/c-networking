#ifndef H_SERVER
#define H_SERVER

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>


void net_error(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}


void start(const char* ip, unsigned short port)
{
    int server_fd, new_socket;
    size_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[1024] = {0};
    char* hello = "hello from server\n";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
        net_error("socket failed on creation\n");
    

    // attach socket to port 8080
    int sock_opt = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (sock_opt)
        net_error("error on socket opt\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // bind port to socket
    int bind_res = bind(server_fd, (struct sockaddr*) &address, sizeof(address));
    if (bind_res < 0)
        net_error("error on socket bind\n");

    int listen_res = listen(server_fd, 5);
    if (listen_res < 0)
        net_error("error on socket listening\n");

    new_socket = accept(server_fd, (struct sockaddr*) &address, &addrlen);
    if (new_socket < 0)
        net_error("error on accepting new connections\n");

    
    while(strcmp(buffer, "$end"))
    {
        valread = read(new_socket, buffer, 1024-1);
        if (valread < 0)
            net_error("error on reading into buffer from socket connection\n");
        
        printf("%s\n", buffer);
    }

    
    send(new_socket, hello, strlen(hello), 0);

    close(new_socket);
    close(server_fd);

}



#endif