#include "../apue.2e/include/apue.h"
#include "../apue.2e/include/error.c"
#include <signal.h>
#include <setjmp.h>
#include <sys/wait.h>

static volatile pid_t   pid;
static sigjmp_buf       jmpbuf;

static void sig_quit(int);
static void sig_alrm(int);
Sigfunc *signal(int, Sigfunc *);

int
main(int argc, char *argv[])
{
    char    buf[MAXLINE];
    int     status;
    int     time = 0;

    if ((argc != 1 && argc != 3) || (argc == 3 && strncmp (argv[1], "-t", 2) != 0)) {
        err_quit("usage: myshell [-t <time>]");
    } else {
        time = atoi(argv[2]);
    }

    if (signal(SIGALRM, sig_alrm) == SIG_ERR || signal(SIGQUIT, sig_quit) == SIG_ERR) {
        err_quit("signal error");
    }

    sigsetjmp(jmpbuf, 1);

    printf("%%");
    while (fgets(buf, MAXLINE, stdin) != NULL) {
        if (buf[MAXLINE - 1] != '\n') {
            buf[MAXLINE - 1] = 0;
        }
        if (time) {
            alarm(time);
        }
        if ((pid = fork()) < 0) {
            err_sys("fork error");
        } else if (pid == 0) {
            execlp("/bin/bash", "sh", "-c", buf, (char *)0);
            err_ret("could not execute: %s", buf);
            exit(127);
        }

        if ((pid = waitpid(pid, &status, 0)) < 0) {
            err_sys("waitpid error");
        }
        if (time) {
            alarm(0);
        }
        printf("%%");
    }

    return 0;
}

Sigfunc *
signal(int signo, Sigfunc *func) {
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (signo == SIGALRM) {
        sigaddset(&act.sa_mask, SIGQUIT);
    } else if (signo == SIGQUIT) {
        sigaddset(&act.sa_mask, SIGALRM);
    }

    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    } else if (signo == SIGQUIT) {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &act, &oact) < 0) {
        return SIG_ERR;
    }

    return(oact.sa_handler);
}

void 
sig_alrm(int signo)
{
    if (pid > 0) {
        kill(pid, SIGKILL);
        pid = 0;
    }
    printf("\n=== BOOM! TIMEOUT ===\n");

    sigset_t pendmask;
    if (sigemptyset(&pendmask) < 0) {
        err_sys("sigemptyset error");
    }
    if (sigpending(&pendmask) < 0) {
        err_sys("sigpending error");
    }
    if (sigismember(&pendmask, SIGQUIT)) {
        signal(SIGQUIT, SIG_IGN);
        signal(SIGQUIT, sig_quit);
    }

    siglongjmp(jmpbuf, 1);

}

void 
sig_quit(int signo)
{
    if (pid > 0) {
        kill(pid, SIGKILL);
        pid = 0;
    }
    printf("\n=== RECV QUIT ===\n");
    alarm(0);

    sigset_t pendmask;
    if (sigemptyset(&pendmask) < 0) {
        err_sys("sigemptyset error");
    }
    if (sigpending(&pendmask) < 0) {
        err_sys("sigpending error");
    }
    if (sigismember(&pendmask, SIGALRM)) {
        signal(SIGALRM, SIG_IGN);
        signal(SIGALRM, sig_alrm);
    }

    siglongjmp(jmpbuf, 1);
    
}
