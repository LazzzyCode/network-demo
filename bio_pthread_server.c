#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 1024

void *handle_socket(void *args);

int main()
{
    int server_sock, client_sock;
    int str_len, i;

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;

    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        perror("socket() error");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(12345);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind() error");
        exit(1);
    }

    if (listen(server_sock, 5) == -1)
    {
        perror("listen() error");
        exit(1);
    }
    while (1)
    {
        printf("waiting for client ...\n");
        client_addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
        printf("new client connected ! \n");
        if (client_sock == -1)
        {
            perror("accept() error");
            exit(1);
        }
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_socket, (void *)&client_sock) != 0)
        {
            perror("pthread create error");
            close(client_sock);
            continue;
        }
        
        pthread_detach(client_thread);
        printf("detach a thread to handle new client \n");
    }
    close(server_sock);

    return 0;
}

void *handle_socket(void *args)
{
    int client_sock = *((int *)args);
    char message[BUF_SIZE];
    int  str_len,recv_len;
    while ((str_len = read(client_sock, message, BUF_SIZE)) != 0)
    {
        message[str_len] = 0;
        printf("Received message: %s\n", message);
        write(client_sock, message, str_len);
    }
    close(client_sock);
}
