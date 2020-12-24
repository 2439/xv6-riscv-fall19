#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int p1[2];
    int p2[2];
    char n1[5];
    char n2[5];

    if (pipe(p1) == -1 || pipe(p2) == -1) {
        printf("pipe error\n");
        exit();
    }
    int pid = fork();
    if(pid < 0){
        printf("fork error\n");
        exit();
    }
    else if (pid == 0) {
        close(p1[1]);
        read(p1[0], n1, sizeof(n1));
        printf("%d: received %s\n", getpid(), n1);
        write(p2[1], "pong", 5);
        close(p2[1]);
        exit();
    } else {
        write(p1[1], "ping", 5);
        close(p1[1]);
        close(p2[1]);
        read(p2[0], n2, sizeof(n2));
        printf("%d: received %s\n", getpid(), n2);
        exit();
    }
}