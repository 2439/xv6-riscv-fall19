#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXARGS 100
#define MAXWORD 50

struct cmd {
    int argc;
    char *argv[MAXARGS];
};

char whitespace[] = " \t\r\n\v";
char args[MAXARGS][MAXWORD];

int getcmd(char *buf, int nbuf);
void parsecmd(struct cmd *ret, char *s);
void runcmd(char *argv[], int argc);
int ifend(char a);
int fork1(void); 
void runonecmd(int argc, char* argv[]);

int main(int argc, char *argv[]) {
    static char buf[100];
    int fd;

    //输入nsh，进入模拟shell
    if (argv[1]) {
        printf("Some input error\n");
        exit(-1);
    }

    //确保打开三个文件描述符,fd为0，1，2
    while((fd = open("console", O_RDWR)) >= 0) {
        if(fd >= 3) {
            close(fd);
            break;
        }
    }

    while(getcmd(buf, sizeof(buf)) >= 0) {
        if(fork1() == 0) {
            struct cmd ret;
            parsecmd(&ret, buf);
            runcmd(ret.argv, ret.argc);
            exit(0);
        }
        wait(0);
    }
    exit(0);
}

//从描述符0中获取输入，无输入返回-1
int getcmd(char *buf, int nbuf) {
    fprintf(2, "@ ");
    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    if(buf[0] == 0) {
        return -1;
    }
    return 0;
}

//将所有单词分段
void parsecmd(struct cmd *ret, char *s) {
    int i=0;
    for(int j=0; !ifend(s[j]); j++) {
        while(strchr(whitespace, s[j]) && !ifend(s[j])) {
            j++;
        }

        ret->argv[i] = &s[j];
        i++;
        while(!strchr(whitespace, s[j]) && !ifend(s[j])) {
            j++;
        }
        s[j] = 0;
        
    }    
    ret->argv[i] = 0;
    ret->argc = i;
}

void runcmd(char *argv[], int argc) {
    int flag = 0;
    for(int i=0; i<argc; i++) {
        if(!strcmp(argv[i], "|")) {
            int p[2];
            flag = 1;
            argv[i] = 0;
            if(pipe(p) < 0) {
                fprintf(2, "pipe");
                exit(-1);
            }
            //第一个|之前的命令
            if(fork1() == 0) {
                close(1);
                dup(p[1]);
                close(p[0]);
                close(p[1]);
                runonecmd(i, argv);
                exit(0);
            }
            //其余命令
            if(fork1() == 0) {
                close(0);
                dup(p[0]);
                close(p[0]);
                close(p[1]);
                runcmd(argv+i+1, argc-i-1);
                exit(0);
            }
            close(p[0]);
            close(p[1]);
            wait(0);
            wait(0);
            break;
        }
    }
    //最后一段
    if(flag == 0){
        runonecmd(argc, argv);
    }
}

//到末尾
int ifend(char a) {
    if (a == '\n' || a == '\0') {
        return 1;
    }
    return 0;
}

int fork1(void) {
    int pid;

    pid = fork();
    if(pid == -1) {
        fprintf(2, "fork");
    }
    return pid;
}

//单程序
void runonecmd(int argc, char* argv[]) {
    for(int i=0; i<argc; i++) {
        if(!strcmp(argv[i], ">")) {
            if(i == argc-1) {
                fprintf(2, "redirect");
                exit(-1);
            }
            close(1);
            open(argv[i+1], O_WRONLY|O_CREATE);
            argv[i] = 0;
        } else if(!strcmp(argv[i], "<")) {
            if(i == argc-1) {
                fprintf(2, "redirect");
                exit(-1);
            }
            close(0);
            open(argv[i+1], O_RDONLY);
            argv[i] = 0;
        }
    }
    exec(argv[0], argv);
}