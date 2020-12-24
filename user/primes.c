#include "kernel/types.h"
#include "user/user.h"

void primes(int *pd);
void fliter(int p, int *pd, int *newpd);

int main(int argc, char *argv[]) {
    int pd[2];
    if (pipe(pd) == -1) {
        printf("pipe error\n");
        exit();
    }

    int pid = fork();
    if(pid < 0){
        printf("fork error\n");
        exit();
    }
    else if (pid == 0) {
        for (int i=2; i<=35; i++) {
            write(pd[1], &i, sizeof(i));
        }
        close(pd[1]);
        exit();
    } else {
        close(pd[1]);
        primes(pd);
        exit();
    }
}

void primes(int *pd) {
    int p;
    int newpd[2];
    if (read(pd[0], &p, sizeof(p))) {
        printf("prime %d\n", p);

        if (pipe(newpd) == -1) {
            printf("pipe error\n");
            exit();
        }

        int pid = fork();
        if(pid < 0){
            printf("fork error\n");
            exit();
        }
        else if(pid == 0) {
            fliter(p, pd, newpd);
            close(newpd[1]);
            exit();
        } else {
            close(pd[0]);
            close(newpd[1]);
            primes(newpd);
        }
    }
}

void fliter(int p, int *pd, int *newpd) {
    int n;
    while (read(pd[0], &n, sizeof(n))) {
        if (n%p != 0) {
            write(newpd[1], &n, sizeof(n));
        }
    }
}

// #include "kernel/types.h"
// #include "user/user.h"

// void redirect(int k, int *pd);
// void primes();
// void fliter(int p);

// int main(int argc, char *argv[]) {
//     int pd[2];
//     if (pipe(pd) == -1) {
//         exit();
//     }

//     if (fork() == 0) {
//         redirect(1, pd);
//         for (int i=2; i<=35; i++) {
//             write(1, &i, sizeof(i));
//         }
//         exit();
//     } else {
//         redirect(0, pd);
//         primes();
//     }
//     exit();
// }

// void redirect(int k, int *pd) {
//     close(k);
//     dup(pd[k]);
//     close(pd[1]);
// }

// void primes() {
//     int p;
//     int pd[2];
//     if (read(0, &p, sizeof(p))) {
//         printf("primes %d\n", p);

//         if (pipe(pd) == -1) {
//             exit();
//         }

//         if (fork() == 0) {
//             redirect(1, pd);
//             fliter(p);
//             exit();
//         } else {
//             redirect(0, pd);
//             primes();
//         }
//     }
// }

// void fliter(int p) {
//     int n;
//     while (read(0, &n, sizeof(n))) {
//         if (n%p != 0) {
//             write(1, &n, sizeof(n));
//         }
//     }
// }