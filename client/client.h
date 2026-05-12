#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define PORT 8080
#define BUFFER_SIZE 56000

char* chat_buffer;
int retry = 6;


void net_error(const char* msg)
{
    perror(msg);
    retry--;

    if (retry == 0)
        exit(EXIT_FAILURE);
}


void* client_read_thread(void* arg)
{
    int* fd = (int*) arg;
    int res_read = 0;
    
    while(res_read >= 0)
    {
        printf("\e[1;1H\e[2J");
        res_read = read(*fd, chat_buffer, BUFFER_SIZE * sizeof(char));
        printf("%s\n", chat_buffer);
    }
    
}


void client_connect(const char* username)
{
    int client_fd;
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
    chat_buffer = calloc(BUFFER_SIZE, sizeof(char));

    send(client_fd, username, strlen(username), 0);


    pthread_t thread;
    pthread_create(&thread, NULL, client_read_thread, (void*)&client_fd);
    pthread_detach(thread);

    while (1)
    {
        memset(input, 0, sizeof(input));
        fgets(input, sizeof(input), stdin);

        if (strncmp(input, "$end", 4) == 0)
            goto out;


        char buffer_send[1024];

        snprintf(buffer_send, sizeof(buffer_send), "%s: %s", username, input);
        send(client_fd, buffer_send, strlen(buffer_send), 0);
    }
    
    
out:
    close(client_fd);
    return;
}
