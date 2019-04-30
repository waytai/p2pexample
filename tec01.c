//p2p点对点聊天多进程版--服务器(信号的使用)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

void handler(int sign)
{
    if(sign==SIGUSR1)
        printf("recv signal!\n");
    exit(0);
}

int main(int arg, char *args[])
{
    //create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket() err");
        return -1;
    }
    //reuse server socket
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))
            == -1)
    {
        perror("setsockopt() err");
        return -1;
    }
    //bind port and ip
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        perror("bind() err");
        return -1;
    }
    //listen 维护两个队列
    if (listen(sockfd, SOMAXCONN) == -1)
    {
        perror("listen() err");
        return -1;
    }
    //accept
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    int conn = accept(sockfd, (struct sockaddr *) &peeraddr, &peerlen);
    if (conn == -1)
    {
        perror("accept() err");
        return -1;
    }
    printf("accept by %s\n", inet_ntoa(peeraddr.sin_addr));
    pid_t pid = 0;
    pid = fork();
    if (pid == -1)
    {
        perror("fork() err");
        return -1;
    }
    //父进程接收客户端信息，打屏
    if (pid > 0)
    {
        char recvbuf[1024] = { 0 };
        int rc = 0;
        while (1)
        {
            rc = read(conn, recvbuf, 1024);
            if (rc < 0)
            {
                perror("read() err");
                break;
            } else if (rc == 0)
            {
                printf("client is closed !\n");
                break;
            }
            //printf("%s\n");
            write(STDOUT_FILENO, recvbuf, rc);
            memset(recvbuf, 0, 1024);
        }
        //发送信号，关闭子进程
        kill(pid,SIGUSR1);
        //关闭客户端连接套接字
        close(conn);
        close(sockfd);
        //等待子进程
        int ret=0;
        while(1)
        {
            ret=wait(NULL);
            printf("子进程pid=%d\n",ret);
            if(ret==-1)
            {
                if(errno==EINTR)
                    continue;
                break;
            }
        }
    }
    //子进程读取用户输入，发送给客户端
    if (pid == 0)
    {
        //安装信号
        struct sigaction act;
        act.sa_handler=handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags=0;
        if(sigaction(SIGUSR1,&act,NULL)==-1)
        {
            printf("sigaction() failed! \n");
            exit(0);
        }
        //关闭服务器监听套接字
        close(sockfd);
        char sendbuf[1024] = { 0 };
        while (1)
        {
            if (read(STDIN_FILENO, sendbuf, 1024) == -1)
            {
                perror("read() err");
                //关闭客户端连接套接字
                close(conn);
                exit(0);
            }
            write(conn, sendbuf, strlen(sendbuf));
            memset(sendbuf, 0, 1024);
        }
    }
    return 0;
}
