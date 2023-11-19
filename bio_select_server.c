#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#define BUF_SIZE 1024

int main()
{
    int server_sock = init_server_sock();
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_sock, &read_fds);

    while (1)
    {
        select_loop(server_sock, read_fds);
    }

    close(server_sock);
    return 0;
}

int select_loop(int server_sock, fd_set read_fds)
{
    int max_fd = server_sock;
    char buffer[BUF_SIZE];

    struct sockaddr_in client_addr;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    socklen_t client_addr_size;
    client_addr_size = sizeof(client_addr);
    // 每次select事件触发后，结构会清空，需要调用select重新传入fds
    int ret = select(server_sock + 1, &read_fds, NULL, NULL, &timeout);
    if (ret == -1)
    {
        perror("select");
        exit(1);
    }
    else if (ret == 0)
    {
        printf("Timeout");
        return -1;
    }
    // server_sock如果触发了IO可读事件，说明有新连接建立
    if (FD_ISSET(server_sock, &read_fds))
    {
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_sock == -1)
        {
            perror("accept");
            return -1;
        }
        FD_SET(client_sock, &read_fds);
        max_fd = client_sock > server_sock ? client_sock : server_sock;
    }
    int fd;
    for (fd = server_sock+1; fd < max_fd + 1; fd++)
    {
        // client_sock如果成功建立,读数据
        if (FD_ISSET(fd, &read_fds))
        {
            printf("read:%d\n", fd);
            int recv_len = read(fd, buffer, BUF_SIZE - 1);
            if (recv_len == -1)
            {
                perror("read");
                close(fd);
                FD_CLR(fd, &read_fds);
                continue;
            }
            else if (recv_len == 0)
            {
                printf("Client disconnected");
                close(fd);
                FD_CLR(fd, &read_fds);
                continue;
            }
            
            buffer[recv_len] = '\0';
            printf("Received: %s\n", buffer);
            send(fd, buffer, recv_len, 0);
        }
    }
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
    server_addr.sin_port = htons(12345);

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
