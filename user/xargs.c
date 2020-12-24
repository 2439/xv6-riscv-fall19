#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    int infoC = 0;
    int inputC;
    int m = 0;
    char buf[MAXARG];
    char *p = buf;
    char block[MAXARG];
    char *info[MAXARG];
    for(int i=1; i<argc; i++){
        info[infoC++] = argv[i];
    }

    while((inputC=read(0, block, sizeof(block))) > 0) {
        for(int l=0; l<inputC; l++) {
            if(block[l] == '\n') {
                buf[m] = 0;
                m=0;
                info[infoC++]=p;
                p=buf;
                info[infoC]=0;
                infoC=argc-1;
                int pid = fork();
                if (pid < 0){
                    printf("fork error\n");
                    exit();
                } else if(pid == 0){
                    exec(argv[1],info);
                } else {
                    wait();
                }
            } else if(block[l] == ' ') {
                buf[m++] = 0;
                info[infoC++] = p;
                p = &buf[m];
            } else {
                buf[m++] = block[l];
            }
        }
    }
    exit();
}