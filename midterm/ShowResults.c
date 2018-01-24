/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ShowResults.c
 * Author: Yunus CEVIK
 *
 * Created on 11 Nisan 2017 Salı, 16:31
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define PATH_MAX 4096
#define PIPE_BUF 4096
#define SHOWRESULTS_FIFO "showFifo"
#define LOGFILE "log/ShowResults.log"
#define TIMER_SERVER "timerServer.txt"
#define SHOWRESULTS "showResult.txt"
/*
 * 
 */
int mainpid=0;
static void closeCTRLC(int signo){

        if(signo==SIGINT){
            kill(mainpid,SIGUSR2);
            fprintf(stderr, "CTRL + C ALDIM KAPANIYORUM \n" );
            unlink(SHOWRESULTS);
            
        }
        else if(signo==SIGUSR2){
            fprintf(stderr, "KILL ALDIM OLUYORUM \n" );
            unlink(SHOWRESULTS);
        }
    exit(0);
        
}
struct incomingInf{
    int inComPid;
    double result1,result2;
    long timeElapsed1,timeElapsed2;
};

int main(int argc, char** argv) {
    
    FILE * filePtr;
    FILE *logPrt;
    FILE * showPtr;
    int fd;
    struct incomingInf information;
    int serverPid;
    union sigval value;
    struct sigaction act;
    int val,val1;;
    if(argc !=1){
        fprintf(stderr,"Usage:./ShowResults\n");
        exit(EXIT_FAILURE);
    }
    filePtr=fopen(TIMER_SERVER,"r");
    if(filePtr==NULL){
        fprintf(stderr, "DOSYA ACILAMADI\n" );
        exit(1);
    }
    fscanf(filePtr,"%d",&val);
    mainpid=val;
    fclose(filePtr);


    showPtr=fopen(SHOWRESULTS,"w");
    fprintf(showPtr, "%ld",(long)getpid() );
    fclose(showPtr);

    act.sa_flags = 0;
    act.sa_handler= closeCTRLC;
    if ((sigemptyset(&act.sa_mask) == -1) ||
            (sigaction(SIGUSR2, &act, NULL) == -1) ||
            (sigaction(SIGINT, &act, NULL) == -1)){
            perror("Failed to set up SIGRTMIN signal");
            return 1;
    }
    

    while(1){
        logPrt=fopen(LOGFILE,"a");

        if (mkfifo(SHOWRESULTS_FIFO, FIFO_PERMS) == -1) { 
            if (errno != EEXIST) {
                fprintf(stderr, "[%ld]:1failed to create named pipe %s: %s\n",
                (long)getpid(), SHOWRESULTS_FIFO, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        fd = open(SHOWRESULTS_FIFO, O_RDWR) ;
        if (fd == -1) {
            fprintf(stderr, "[%ld]:2failed to open named pipe %s for write: %s\n",
            (long)getpid(), SHOWRESULTS_FIFO, strerror(errno));
            exit(EXIT_FAILURE);
        }
        read(fd,&information,sizeof(struct incomingInf));
            fprintf(stdout,"PID: %d\tResult1: %f\tResult2: %f\n",information.inComPid,information.result1,information.result2);
            fprintf(logPrt,"PID: %d\n",information.inComPid);
            fprintf(logPrt,"Result1: %f, TimeElapsed1: %ld\n",information.result1,information.timeElapsed1);
            fprintf(logPrt,"Result2: %f, TimeElapsed2: %ld\n",information.result2,information.timeElapsed2);
            fprintf(logPrt,"\n");
        
        close(fd);
        fclose(logPrt);
    }
    
    
    return (EXIT_SUCCESS);
}

