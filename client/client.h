#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#define PORT 8080

int retry = 6;


void net_error(const char* msg)
{
    perror(msg);
    retry--;

    if (retry == 0)
        exit(EXIT_FAILURE);
}

void client_connect()
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;

    char buffer[1024] = {0};

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        printf("Socket creation error\n");
        return;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    int net_ip_res = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    if (net_ip_res <= 0)
    {
        printf("Invalid address used.\n");
        return;
    }

    int connect_res = connect(client_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if (connect_res < 0)
        net_error("Connection failed.\nTrying to connect again.");
    
    char input[100];
    while (1)
    {
        if (strncmp(input, "$end", 4) == 0)
            goto out;

        memset(input, 0, sizeof(input));
        fgets(input, sizeof(input), stdin);
        send(client_fd, input, strlen(input), 0);

    }
    
    
    // valread = read(client_fd, buffer, 1024 - 1);

out:
    close(client_fd);

    return;
}
