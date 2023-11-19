#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <netinet/in.h>
#define BUF_SIZE 1024

int main() {
    int sock;
    char message[BUF_SIZE];
    int str_len;

    struct sockaddr_in server_addr;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket() error");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(1234);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect() error");
        exit(1);
    }

    strcpy(message, "helloworld");
    pid_t pid = fork();
    // int pid = getpid();
    //发送SIGUSR1信号
    kill(pid, SIGUSR1);
    str_len = read(sock, message, BUF_SIZE - 1);
    message[str_len] = 0;
    printf("Received message: %s\n", message);

    close(sock);

    return 0;
}

