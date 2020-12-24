#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *filename);
char* fmtname(char *path);

int main(int argc, char *argv[]) {
    if(argc < 3){
        printf("find path filename\n");
        exit();
    }
    find(argv[1], argv[2]);
    exit();
}

void find(char *path, char *filename) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0) {
        printf("find: cannot open %s\n", path);
        close(fd);
        return;
    }

    if(fstat(fd, &st) < 0) {
        printf("find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type) {
        case T_FILE:
            if(strcmp(fmtname(path), filename) == 0) {
                printf("%s\n", path);
            }
            break;
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("ls: path too long\n");
                break;
            }

            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)) {
                if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0){
                    continue;
                }
                memmove(p, de.name, strlen(de.name));
                p[strlen(de.name)] = 0;
                find(buf, filename);
            }
            break;
    }
    close(fd);
}

char *fmtname(char *path){
    static char buf[DIRSIZ+1];
    char *p;

    for(p=path+strlen(path); p>=path && *p!='/'; p--);
    p++;

    memmove(buf, p, strlen(p)+1);
    return buf;
}