
/* 
 * File:   timerServer.c
 * Author: Yunus CEVIK
 *
 * Created on 11 Nisan 2017 Salı, 16:28
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define PID_NAME_SIZE 10
#define MATRIX_SIZE 255
#define SIZE 30
#define LOG_DIRECTORY "log"
#define LOGFILE "log/timerServer.log"
#define TIMER_SERVER "timerServer.txt"
#define SHOWRESULTS "showResult.txt"
#define NANOSECONDS 1000000000000L
#define MILLION 1000000L
#define BILLION 1000000000L
/*
 * 
 */
 static volatile sig_atomic_t doneflag=0;
 int  clientPid[SIZE],pidCount=0;
 int isMain=1,showid=0;
 char serverFIFO[SIZE];
double determinant(double a[][MATRIX_SIZE], int k);
void randomMatrix(double matrix[][MATRIX_SIZE],int size);
void pidFifo(int pidNameFifo,int timerSer);


static void closeCTRLC(int signo){
    int i;
    sigset_t sigset;
    
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGUSR2);
    if(isMain==1){
        if(signo==SIGINT){
            fprintf(stderr, "CTRL + C ALDIM KAPANIYORUM \n" );

        }
        else if(signo==SIGUSR2){
            fprintf(stderr, "KILL ALDIM OLUYORUM \n" );
        }



        for(i=0 ; i<pidCount;i++){
            kill(clientPid[i],SIGUSR2);
            kill(showid,SIGUSR2);
        }

        while(wait(NULL)!=-1); 
        unlink(TIMER_SERVER) ;
        unlink(serverFIFO);
        unlink("showFifo");
        remove("SeeWhat");
        remove("timerServer");
        remove("ShowResults");
        exit(1);
    }
    
}
static void requestHandler(int signo, siginfo_t* info, void *context){
    int val;
    val = info->si_value.sival_int;
    clientPid[pidCount] = val;
    pidCount++;
    doneflag=1;
        
}
struct matrixInf{
    double matrix[MATRIX_SIZE][MATRIX_SIZE];
    int size;
    double orjDetValue;
};
struct matrixInf information;
int main(int argc, char** argv) 
{
    FILE * showPtr;
    FILE *filePtr;  
    int fd,client;
    double d;
    char clientPidFifo[PID_NAME_SIZE];
    struct sigaction act;
    struct sigaction act2;
    int mainPid;
    char path[PATH_MAX];
    getcwd(path,PATH_MAX);
    strcat(path,"/");
    strcat(path,LOG_DIRECTORY);
    system("rm -rf log/");
    mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    sigset_t newMask, oldMask;
    struct itimerspec newVal;
    struct itimerspec currVAl;
    struct sigevent event;
    timer_t timeNum;
    int flag=1,val1;

    if(argc !=4){
        fprintf(stderr,"Usage:./timerServer <ticks in mSec> <n> <mpipename>\n");
        exit(EXIT_FAILURE);
    }
    strcpy(serverFIFO,argv[3]);
    information.size=(atoi(argv[2])*2);

    mainPid = getpid();
    
    filePtr = fopen(TIMER_SERVER,"w");
    fprintf(filePtr,"%d",mainPid);
    fclose(filePtr);



    



    event.sigev_notify = SIGEV_NONE;

    if (timer_create(CLOCK_PROCESS_CPUTIME_ID, &event, &timeNum) == -1) {
        perror("Failed to create a timer based on CLOCK_REALTIME");
        return -1;
    }
    
    newVal.it_value.tv_sec = 0;
    newVal.it_value.tv_nsec = atoi(argv[1])*MILLION;
    newVal.it_interval.tv_sec = 0;
    newVal.it_interval.tv_nsec = atoi(argv[1])*MILLION;

    if (newVal.it_interval.tv_nsec >= BILLION) {
        newVal.it_interval.tv_sec++;
        newVal.it_interval.tv_nsec -= BILLION;
    }

    if (timer_settime(timeNum, 0, &newVal, NULL) == -1) {
        perror("Failed to set interval timer");
        return -1;
    }
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &requestHandler;
    if ((sigemptyset(&act.sa_mask) == -1) ||
            (sigaction(SIGRTMIN, &act, NULL) == -1)) {
            perror("Failed to set up SIGRTMIN signal");
            return 1;
    }
    act2.sa_flags = 0;
    act2.sa_handler = closeCTRLC;
    if ((sigemptyset(&act2.sa_mask) == -1) ||
            (sigaction(SIGUSR2, &act2, NULL) == -1) ||
            (sigaction(SIGINT, &act2, NULL) == -1)){
            perror("Failed to set up SIGUSR2 signal");
            return 1;
    }
    
    /*------------------------mainfifo-------------------------------------*/            
    if (mkfifo(argv[3], FIFO_PERMS) == -1) { 
        if (errno != EEXIST) {
            fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
            (long)getpid(), argv[3], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    fd = open(argv[3], O_WRONLY) ;

    if (fd == -1) {
        fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
        (long)getpid(), argv[3], strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    write(fd , &mainPid , sizeof(int));
    /*--------------------------------------------------------------------*/ 
    
        while(1){
            if((showPtr=fopen(SHOWRESULTS,"r"))!=NULL){
                fscanf(showPtr,"%d",&val1);
                showid=val1;
            }
            
            timer_gettime(timeNum, &currVAl);  
            
            if(currVAl.it_value.tv_nsec == 0){    
                
                if (doneflag==1){
                    doneflag=0;
                    pidFifo(clientPid[pidCount - 1],mainPid);
                }
            }
        }
        

        
        
            
    return (EXIT_SUCCESS);
}
void pidFifo(int pidNameFifo,int timerSer){
    FILE *logPtr;
    pid_t pid;
    int ReadfifoDescripter;
    int WritefifoDescripter;
    char ReadFifo[SIZE];
    char WriteFifo[SIZE];
    char clientFifoName[SIZE];
    int clientPidControl;
    
    double detValue=0.0;
    struct timespec timers;
    struct timeval tStart;
    struct timeval tEnd;
    long timeDif;
    
    clock_gettime(CLOCK_MONOTONIC, &timers);
    sprintf(ReadFifo , "read_" );
    sprintf(WriteFifo , "write_");

    sprintf(clientFifoName,"%d",pidNameFifo);
    strcat(ReadFifo ,clientFifoName);
    strcat(WriteFifo,clientFifoName);



    if (mkfifo(ReadFifo, FIFO_PERMS) == -1) { 
        if (errno != EEXIST) {
            fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
            (long)getpid(), ReadFifo, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (mkfifo(WriteFifo, FIFO_PERMS) == -1) { 
        if (errno != EEXIST) {
            fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
            (long)getpid(), WriteFifo, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    ReadfifoDescripter = open(WriteFifo, O_RDONLY) ;

    WritefifoDescripter = open(ReadFifo, O_WRONLY) ;


    if (ReadfifoDescripter == -1) {
        fprintf(stderr, "[%ld]:1failed to open named pipe %s for write: %s\n",
        (long)getpid(), ReadFifo, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (WritefifoDescripter == -1) {
        fprintf(stderr, "[%ld]:2failed to open named pipe %s for write: %s\n",
        (long)getpid(), WriteFifo, strerror(errno));
        exit(EXIT_FAILURE);
    }


    pid = fork();

    if(pid == 0)
    {
        isMain=0;
        srand((time_t)timers.tv_nsec);
        while(1)
        {   

            logPtr=fopen(LOGFILE,"a");
            fprintf(logPtr,"\nTimerServer_PID: %d\n",timerSer );
            gettimeofday(&tStart,NULL);
            randomMatrix(information.matrix,information.size);
            gettimeofday(&tEnd,NULL);
            timeDif = 1000*(tEnd.tv_sec - tStart.tv_sec) + (tEnd.tv_usec - tStart.tv_usec)/1000.0;
            detValue=determinant(information.matrix,information.size);
            fprintf(logPtr,"Generation Time: %ld\n",timeDif );
            fprintf(logPtr,"Determinant: %f\n",detValue );
            information.orjDetValue=detValue;
            if(detValue==0.0){
                printf("determinant sifir yeni matrix olusturuluyor\n");
                randomMatrix(information.matrix,information.size);
            }
            else{
                write(WritefifoDescripter , &information , sizeof(struct matrixInf));
                read(ReadfifoDescripter , &clientPidControl, sizeof(int));  
            }
            fclose(logPtr);
        }

    }

}
void randomMatrix(double matrix[MATRIX_SIZE][MATRIX_SIZE],int size){
    int i,j;
    for (i = 0;i < size; i++){
        for (j = 0;j < size; j++){
            matrix[i][j]=rand()%21+(-10);  
        }

        
    }
}
double determinant(double a[MATRIX_SIZE][MATRIX_SIZE], int k)
{
    double s = 1, det = 0, b[MATRIX_SIZE][MATRIX_SIZE];
    int i, j, m, n, c;
    if (k == 1)
        return (a[0][0]);
    else
    {
        det = 0;
        for (c = 0; c < k; c++){
            m = 0;
            n = 0;
            for (i = 0;i < k; i++){
                for (j = 0 ;j < k; j++){
                    b[i][j] = 0;
                    if (i != 0 && j != c){
                        b[m][n] = a[i][j];
                        if (n < (k - 2))
                            n++;
                        else
                        {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            det = det + s * (a[0][c] * determinant(b, k - 1));
            s = -1 * s;
        }
    }
    return (det);
}

