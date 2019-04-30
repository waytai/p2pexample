//p2p点对点聊天多进程版--客户端
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int arg, char *args[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket() err");
        return -1;
    }
    //connect
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        perror("connect() err");
        return -1;
    }
    pid_t pid = 0;
    pid = fork();
    if (pid == -1)
    {
        perror("fork() err");
        return -1;
    }
    int rc = 0;
    char buf[1024] = { 0 };
    //子进程接收信息
    if (pid == 0)
    {
        while (1)
        {
            rc = read(sockfd, buf, 1024);
            if (rc < 0)
            {
                perror("read err");
                close(sockfd);
                exit(0);
            } else if (rc == 0)
            {
                printf("server closed!\n");
                close(sockfd);
                exit(0);
            }
            write(STDOUT_FILENO, buf, rc);
            memset(buf, 0, 1024);
        }
    }
    //父进程发送数据
    if (pid > 0)
    {
        while (1)
        {
            if (read(STDIN_FILENO, buf, 1024) == -1)
            {
                perror("read() err");
                close(sockfd);
                exit(0);
            }
            write(sockfd, buf, strlen(buf));
            memset(buf, 0, 1024);
        }
    }
    return 0;
}
