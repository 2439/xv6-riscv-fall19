#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[])
{
    int infoC = 0;      // xargs后面信息计数
    int inputC;         // 标准输入长度
    int m = 0;          // buf长度
    char buf[MAXARG];   // 当前输入内容
    char *p = buf;      // buf内容最后一位的后一位
    char block[MAXARG]; // 当前标准输入内容
    char *info[MAXARG]; // xargs后面的信息指针，MAXARG：xargs后最多的单词
    // argv[0] = "xargs"，argv[1]：命令，argv[2..]：命令信息
    // 初始化info
    for (int i = 1; i < argc; i++)
    {
        info[infoC++] = argv[i];
    }
    // 标准输入，每一行输入调用命令
    while ((inputC = read(0, block, sizeof(block))) > 0)
    {
        for (int l = 0; l < inputC; l++) // inputC：标准输入长度
        {
            if (block[l] == '\n')
            {                      // 当前字符换行，调用命令
                buf[m] = 0;        // 最后一个字符串
                info[infoC++] = p; // 最后一个非0参数
                info[infoC] = 0;   // 命令最后一个参数为0

                m = 0; // 重置，为了下一行的输入
                p = buf;
                infoC = argc - 1;

                int pid = fork();
                if (pid < 0)
                {
                    printf("fork error\n");
                    exit();
                }
                else if (pid == 0)
                {
                    exec(argv[1], info);
                }
                else
                {
                    wait(); // 等待子进程结束
                }
            }
            else if (block[l] == ' ')
            {                 // 当前字符空格，命令信息后面多一项
                buf[m++] = 0; // 字符串
                info[infoC++] = p;
                p = &buf[m];
            }
            else
            {
                buf[m++] = block[l];
            }
        }
    }
    exit();
}