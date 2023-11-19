/** 编译命令： gcc aio_server.c -lrt -o aio_server
 *  运行：./aio_server
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <aio.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int listen_sock, client_sock, n;
    struct aiocb cb;
    char buffer[BUFFER_SIZE];

    // 创建监听套接字
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置套接字选项
    int reuse = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 绑定套接字
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(12345);
    if (bind(listen_sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 监听套接字
    if (listen(listen_sock, 10) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for incoming connections...\n");

    // 等待客户端连接
    client_sock = accept(listen_sock, (struct sockaddr *)NULL, NULL);
    if (client_sock == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Client connected: %d\n", client_sock);

    // 初始化AIO控制块
    memset(&cb, 0, sizeof(cb));
    cb.aio_fildes = client_sock;
    cb.aio_buf = buffer;
    cb.aio_nbytes = BUFFER_SIZE;
    cb.aio_offset = 0;

    // 读取客户端发送的数据
    while ((n = aio_read(&cb)) == -1 && errno == EINPROGRESS)
    {
        // 等待异步读取完成
        sleep(1);
    }

    if (n == -1)
    {
        perror("aio_read error \n");
        exit(EXIT_FAILURE);
    }
    else
    {
        // 循环等待io处理完成
        while (aio_error(&cb) == EINPROGRESS)
        {
            printf("wait io process ...\n");
            sleep(1);
        }

        // 判断请求是否成功
        if (aio_return(&cb) > 0)
        {
            printf("Received: %s\n", buffer);
            write(client_sock, buffer, BUFFER_SIZE);
        }
        else
        {
            printf("IO error\n", buffer);
        }
    }
}