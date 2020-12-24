#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int minNum = 1;
    for (int i = 0; i < strlen(argv[1]) - 1; i++)
    { // 当前输入的最小数值，abcd对应4位，最小1000
        minNum *= 10;
    }
    if (atoi(argv[1]) < minNum)
    { // 确保输入为整数
        printf("Some input error\n");
        exit();
    }
    else
    {
        printf("%s ", argv[0]);
        printf("%s\n", argv[1]);
        printf("(nothing happens for a little while)\n");
        sleep(atoi(argv[1]));
        exit();
    }
}