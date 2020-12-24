#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int p1[2];  // 主进程->子进程
    int p2[2];  // 子进程->主进程
    char n1[5]; // 子进程读
    char n2[5]; // 主进程读

    // 管道创建，p[0] 读，p[1]写
    if (pipe(p1) == -1 || pipe(p2) == -1)
    {
        printf("pipe error\n");
        exit();
    }
    // 进程创建
    int pid = fork();
    if (pid < 0)
    {
        printf("fork error\n");
        exit();
    }
    else if (pid == 0)
    {                                // 子进程
        close(p1[1]);                // 关闭子进程p1写端口
        read(p1[0], n1, sizeof(n1)); // p1管道中无数据时，阻塞
        printf("%d: received %s\n", getpid(), n1);
        write(p2[1], "pong", 5);
        close(p2[1]);
        exit();
    }
    else
    {
        write(p1[1], "ping", 5);
        close(p1[1]);
        close(p2[1]);                // 关闭主进程p2写端口
        read(p2[0], n2, sizeof(n2)); // p2管道中无数据时，阻塞
        printf("%d: received %s\n", getpid(), n2);
        exit();
    }
}