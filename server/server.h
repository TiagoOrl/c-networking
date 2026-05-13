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
#include <pthread.h>
#include <stdint.h>

#define CHAT_BUFFER_SIZE 56000
#define CHAT_CLIENT_MAX_CONN 100

struct client 
{
    int id;
    int client_fd;
    char name[40];
    unsigned char is_connected;
};

char* chat_buffer;
pthread_mutex_t lock;
struct client* clients;


void net_error(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}


int server_accept_new_conn(struct sockaddr_in* address, int server_fd, socklen_t * addrlen)
{
    int client_fd = accept(server_fd, (struct sockaddr*) address, addrlen);
    if (client_fd < 0) {
        if (errno == EINTR) {
            return -2; // Specialized code to tell the loop: "just try again"
        }
        return client_fd;
    }

    return client_fd;
}


void server_broadcast_all(char* c_buffer, unsigned long size)
{
    for (int i = 0; i < CHAT_CLIENT_MAX_CONN; i++)
    {
        if (clients[i].is_connected == 1)
        {
            ssize_t send_res = send(clients[i].client_fd, c_buffer, size, 0);
            if (send_res < 0 )
                printf("%ld: error on sending message/n", send_res);
        }
    }
}


void* server_client_conn_thread(void* arg)
{

    char in_buffer[1024];
    char tmp_buffer[80];
    struct client* client = (struct client*) arg;

    int n = read(client->client_fd, in_buffer, sizeof(in_buffer) - 1);

    if (n > 0) 
        in_buffer[n] = '\0'; 
    
    strncpy(client->name, in_buffer, 20);
    snprintf(tmp_buffer, 80, "(%i):%s connected\n", client->id, client->name);
    
    pthread_mutex_lock(&lock);
    strncat(chat_buffer, tmp_buffer, strlen(tmp_buffer));
    server_broadcast_all(chat_buffer, strlen(chat_buffer));
    pthread_mutex_unlock(&lock);


    printf("\e[1;1H\e[2J");
    printf("%s\n", chat_buffer);
    

    while (1)
    {
        memset(in_buffer, 0, sizeof(in_buffer));
        size_t res_read = read(client->client_fd, in_buffer, sizeof(in_buffer) - 1);

        if (res_read <= 0)
        {
            memset(tmp_buffer, 0, sizeof(tmp_buffer));
            snprintf(tmp_buffer, 80, "(%d)%s disconnected\n", client->id, client->name);

            pthread_mutex_lock(&lock);

            strncat(chat_buffer, tmp_buffer, strlen(tmp_buffer));

            printf("\e[1;1H\e[2J");
            printf("%s\n", chat_buffer);

            pthread_mutex_unlock(&lock);
            
            break;
        }
        
        if (in_buffer[0] != 0)
        {
            printf("\e[1;1H\e[2J");
            pthread_mutex_lock(&lock);

            if (strlen(in_buffer) + strlen(chat_buffer) < CHAT_BUFFER_SIZE - 1)
            {
                strncat(chat_buffer, in_buffer, strlen(in_buffer));
                server_broadcast_all(chat_buffer, strlen(chat_buffer));
            }
            
            printf("%s\n", chat_buffer);

            pthread_mutex_unlock(&lock);
            
        }
    }

    pthread_mutex_lock(&lock);
    client->is_connected = 0;
    pthread_mutex_unlock(&lock);

    close(client->client_fd);
    return NULL;
}


int server_get_free_conn_slot()
{
    int found = -1;
    for (int i = 0;i < CHAT_CLIENT_MAX_CONN; i++)
    {
        if (clients[i].is_connected == 0)
        {
            found = i;
            break;
        }
    }

    return found;
}


void server_init_clients()
{
    clients = calloc(CHAT_CLIENT_MAX_CONN, sizeof(struct client));

    for(int i = 0;i < CHAT_CLIENT_MAX_CONN; i++)
    {
        clients[i].is_connected = 0;
    }
}


void server_start(unsigned short port)
{
    int server_fd;
    size_t res_read;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int opt = 1;
    char buffer[1024] = {0};
    chat_buffer = calloc(CHAT_BUFFER_SIZE, sizeof(char));
    pthread_mutex_init(&lock, NULL);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_init_clients();

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

    
    while(1)
    {
        
        
        int client_fd = server_accept_new_conn(&address, server_fd, &addrlen);
        if (client_fd < 0)
        {
            printf("$error: on accepting new client connection: %d\n", client_fd);
            continue;
        }

        int client_slot = server_get_free_conn_slot();
        if (client_slot < 0)
        {
            printf("max number of concurrent clients reached...\n");
            close(client_fd);
            continue;
        }

        clients[client_slot].id = client_slot;
        clients[client_slot].client_fd = client_fd;
        clients[client_slot].is_connected = 1;


        pthread_t thread;
        
        pthread_create(&thread, NULL, server_client_conn_thread, (void*)&clients[client_slot]);
        pthread_detach(thread);
    }


    pthread_mutex_destroy(&lock);
    close(server_fd);
}



#endif