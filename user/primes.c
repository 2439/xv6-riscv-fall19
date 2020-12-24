#include "kernel/types.h"
#include "user/user.h"

void primes(int *pd);
void fliter(int p, int *pd, int *newpd);

int main(int argc, char *argv[])
{
    int pd[2];
    // 创建管道
    if (pipe(pd) == -1)
    {
        printf("pipe error\n");
        exit();
    }

    int pid = fork();
    if (pid < 0)
    {
        printf("fork error\n");
        exit();
    }
    else if (pid == 0)
    { // 将所有需要判断的数据放入管道
        for (int i = 2; i <= 35; i++)
        {
            write(pd[1], &i, sizeof(i));
        }
        close(pd[1]);
        exit();
    }
    else
    {
        close(pd[1]); // 管道写端口都关闭时，才认为管道关闭
        primes(pd);   // 素数判断函数
        exit();
    }
}

/**
 * 功能: 递归输出pd管道中，所有素数
 * 参数 pd: 读入管道
 */
void primes(int *pd)
{
    int p;        // 管道中第一个数
    int newpd[2]; // 新管道，写入pd管道中不能整除p的数
    if (read(pd[0], &p, sizeof(p)))
    {
        printf("prime %d\n", p); // 管道中第一个数，一定为素数

        if (pipe(newpd) == -1)
        { //创建新管道
            printf("pipe error\n");
            exit();
        }

        int pid = fork();
        if (pid < 0)
        {
            printf("fork error\n");
            exit();
        }
        else if (pid == 0)
        {
            fliter(p, pd, newpd); // 将pd中剩余不能整除p的数写入newpd
            close(newpd[1]);
            exit();
        }
        else
        {
            close(pd[0]);
            close(newpd[1]);
            primes(newpd); // newpd中素数判断
        }
    }
}

/**
 * 功能: 将pd中剩余不能整除p的数写入newpd
 * 参数 pd: 读入管道
 * 参数 p: 需要整除的数字
 * 参数 newpd：写入管道
 */
void fliter(int p, int *pd, int *newpd)
{
    int n;
    while (read(pd[0], &n, sizeof(n)))
    {
        if (n % p != 0)
        {
            write(newpd[1], &n, sizeof(n));
        }
    }
}