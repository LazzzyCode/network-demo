#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define _XOPEN_SOURCE 700

#define PORT 8080
#define BUFFER_SIZE 1024
void signal_handler(int signum);
int server_sock;
int client_sock;
int main(){
    server_sock = init_server_sock();
    char buffer[BUFFER_SIZE] = {0};
    ssize_t valread;
    //1. 注册信号处理函数
    signal(SIGUSR1, signal_handler);

    while (1)
    {
        printf("Waiting for a connection...\n");  
        client_sock = accept(server_sock, (struct sockaddr*) NULL, NULL);  
        if (client_sock == -1) {  
            error("accept error\n");  
        } else {  
            printf("Connection accepted\n");  
        }  
    }

    close(server_sock);
    return 0;
}

//信号处理函数
void signal_handler(int signum){
   
    printf("收到信号 %d !\n", signum);
    char helloworld[] = "Helloworld";  
    write(client_sock, helloworld, strlen(helloworld));  
    close(client_sock);  
}

int init_server_sock()
{
    int server_sock, max_fd, min_fd;
    struct sockaddr_in server_addr;
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(1234);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_sock, 5) == -1)
    {
        perror("listen");
        exit(1);
    }
    return server_sock;
}