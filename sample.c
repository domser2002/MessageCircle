#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#define ERR(source)(fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))
#define PROC_NUMBER 3

void wait_children()
{
    while(1)
    {
        pid_t pid;
        pid = waitpid(0,NULL,WNOHANG);
        if(pid == 0)
            break;
        if(pid < 0)
        {
            if(errno == ECHILD) break;
            ERR("waitpid");
        }
    }
}

void closerw(int readfd,int writefd)
{
    if(close(readfd) == -1)
        ERR("close");
    if(close(writefd) == -1)
        ERR("close");
}

void child_work(int readfd,int writefd)
{
    srand(getpid());
    while(1)
    {
        int n;
        ssize_t ret = read(readfd,&n,sizeof(int));
        if(ret == -1)
            ERR("write");
        if(ret == 0)
        {
            closerw(readfd,writefd);
            break;
        }
        printf("child with pid %d read %d\n",getpid(),n);
        if(n == 0)
        {
            closerw(readfd,writefd);
            break;
        }
        int k = -10 + rand() % 21;
        n *= k;
        if(write(writefd,&n,sizeof(int)) == -1)
            ERR("write");
        printf("child with pid %d wrote %d\n",getpid(),n);
        sleep(1);
    }
}

void parent_work(int readfd,int writefd)
{
    int n = 1;
    if(write(writefd,&n,sizeof(int)) == -1)
        ERR("write");
    child_work(readfd,writefd);
    wait_children();
}

void create_children(int fd[PROC_NUMBER][2])
{
    pid_t pid;
    for(int i=0;i<PROC_NUMBER-1;i++)
    {
        pid = fork();
        switch (pid)
        {
            case -1:
                ERR("fork");
            case 0:
                for(int j=0;j<PROC_NUMBER;j++)
                {
                    if(j != i) 
                    {
                        if(close(fd[j][0]) == -1)
                            ERR("close");
                    }
                    if(j != (i+1)%PROC_NUMBER) 
                    {
                        if(close(fd[j][1]) == -1)
                            ERR("close");
                    }
                }
                child_work(fd[i][0],fd[(i+1)%PROC_NUMBER][1]);
                exit(EXIT_SUCCESS);
            default:
                break;
        }
    }
    for(int i=1;i<PROC_NUMBER-1;i++)
    {
        if(close(fd[i][0]) == -1)
            ERR("close");
        if(close(fd[i][1]) == -1)
            ERR("close");
    }
    if(close(fd[0][0]) == -1)
        ERR("close");
    if(close(fd[PROC_NUMBER-1][1]) == -1)
        ERR("close");
    parent_work(fd[PROC_NUMBER-1][0],fd[0][1]);
}

int main(void)
{
    int fd[PROC_NUMBER][2];
    for(int i=0;i<PROC_NUMBER;i++)
    {
        if(pipe(fd[i]) == -1)
            ERR("pipe");
    }
    create_children(fd);
    return EXIT_SUCCESS;
}
