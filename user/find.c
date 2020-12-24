#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *filename);
char *fmtname(char *path);

int main(int argc, char *argv[])
{
    if (argc < 3)
    { // find 目录树 文件名
        printf("find path filename\n");
        exit();
    }
    find(argv[1], argv[2]);
    exit();
}

/**
 * 功能：找path目录树下寻找的filename的文件
 * 参数 path：目录树
 * 参数 filename：文件
 */
void find(char *path, char *filename)
{
    char buf[512], *p;
    int fd; // path文件句柄
    struct dirent de;
    struct stat st; // path文件信息

    // 打开目录
    if ((fd = open(path, 0)) < 0) // 打开文件，读模式
    {
        printf("find: cannot open %s\n", path);
        close(fd);
        return;
    }
    // 文件信息stat st
    if (fstat(fd, &st) < 0)
    {
        printf("find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    // 文件寻找，文件：对比名字，文件夹：递归查找
    switch (st.type)
    {
    case T_FILE:
        if (strcmp(fmtname(path), filename) == 0)
        {
            printf("%s\n", path);
        }
        break;
    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) // 1：/,DIRSIZ：文件名长，1：结束符
        {
            printf("ls: path too long\n");
            break;
        }

        strcpy(buf, path);
        p = buf + strlen(buf); // 指向path末尾
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de)) // 读取目录项
        {
            if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            { // de.inum == 0：根目录
                continue;
            }
            memmove(p, de.name, strlen(de.name));
            p[strlen(de.name)] = 0;
            find(buf, filename); // 递归查找
        }
        break;
    }
    close(fd);
}

/**
 * 功能：解析路径，得到对应的的文件名
 */
char *fmtname(char *path)
{
    static char buf[DIRSIZ + 1]; // 文件名长DIRSIZ，结束符1
    char *p;

    for (p = path + strlen(path); p >= path && *p != '/'; p--) // 从文件末尾倒退，直到第一个/
        ;
    p++;

    memmove(buf, p, strlen(p) + 1);
    return buf;
}