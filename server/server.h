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

int server_accept_new_conn(struct sockaddr_in* address, int server_fd, socklen_t * addrlen)
{
    int client_fd = accept(server_fd, (struct sockaddr*) address, addrlen);
    if (client_fd < 0)
        net_error("error on accepting new connections\n");

    printf("$new: client connected\n");
    return client_fd;
}


void server_start(unsigned short port)
{
    int server_fd;
    size_t res_read;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int opt = 1;
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



    int client_fd = server_accept_new_conn(&address, server_fd, &addrlen);

    
    while(1)
    {
        memset(&buffer, 0, sizeof(buffer));
        res_read = read(client_fd, buffer, 1024-1);

        if (res_read <= 0)
        {
            net_error("$error: client disconnected abruptly, restarting\n");
            close(client_fd);
            
            client_fd = server_accept_new_conn(&address, server_fd, &addrlen);
            continue;
        }


        if (strncmp(buffer, "$end", 4) == 0)
        {
            printf("$end: connection close request by client.\n");
            close(client_fd);
            client_fd = server_accept_new_conn(&address, server_fd, &addrlen);
            continue;
        }

        
        if (buffer[0] != 0)
            printf("%s", buffer);
    }

    
    // send(new_socket, hello, strlen(hello), 0);

    close(client_fd);
    close(server_fd);

}



#endif