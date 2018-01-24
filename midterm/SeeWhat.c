

/* 
 * File:   SeeWhat.c
 * Author: Yunus CEVIK
 *
 * Created on 11 Nisan 2017 Salı, 16:32
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
#include <sys/types.h>
#include <math.h>
#include <signal.h>
 #include <sys/time.h>
#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define SHOWRESULTS_FIFO "showFifo"
#define MATRIX_SIZE 255
#define SIZE 30
#define NANOSECONDS 1000000000000L
#define MILLION 1000000L
#define BILLION 1000000000L
/*
 * 
 */
char rClient[SIZE];
    char wClient[SIZE];
 int mainServer=0,isMain=1;
double determinant(double a[][MATRIX_SIZE], int k);
void cofactor(double num[][MATRIX_SIZE], int f);
void transpose(double num[][MATRIX_SIZE], double fac[][MATRIX_SIZE], int r);
void convolutionMatrix2D(double matrix[][MATRIX_SIZE],int k);
void shiftedFourDivision(double mainMatrix[][MATRIX_SIZE],
                         double firstMatrix[][MATRIX_SIZE],
                         double secondMatrix[][MATRIX_SIZE],
                         double thirdMatrix[][MATRIX_SIZE],
                         double fourthMatrix[][MATRIX_SIZE],int size);


static void closeCTRLC(int signo){
    int i;
    sigset_t sigset;
    
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGUSR2);

    if(isMain==1){
        if(signo==SIGINT){
            kill(mainServer,SIGUSR2);
            fprintf(stderr, "CTRL + C ALDIM KAPANIYORUM \n" );
            sigprocmask(SIG_BLOCK,&sigset,NULL);
            wait(NULL);
            unlink(wClient);
            unlink(rClient);
            exit(1);

        }
        else if(signo==SIGUSR2){
            fprintf(stderr, "KILL ALDIM OLUYORUM \n" );
            wait(NULL); 
            unlink(wClient);
            unlink(rClient);
            exit(1);
        }
        
    }
    
}

struct matrixInf{
    
    double matrixArr[MATRIX_SIZE][MATRIX_SIZE];
    int size;
    double orjDetValue;

};
struct incomingInf{
    int inComPid;
    double result1,result2;
    long timeElapsed1,timeElapsed2;
};
int main(int argc, char** argv) {
    int i,j,k,l;
    double shiftedInverse[MATRIX_SIZE][MATRIX_SIZE], shiftedConvolution[MATRIX_SIZE][MATRIX_SIZE], d=0.0;
    double aMat[MATRIX_SIZE][MATRIX_SIZE],bMat[MATRIX_SIZE][MATRIX_SIZE],cMat[MATRIX_SIZE][MATRIX_SIZE],dMat[MATRIX_SIZE][MATRIX_SIZE];
    char ReadFifo[SIZE];
    char WriteFifo[SIZE];
    int fd,fdp[2],mainPid;
    FILE * logPtr;
    char pidFifoName[SIZE],seeWhatLog[SIZE];
    struct incomingInf information;
    struct matrixInf matrix;
    union sigval value;
    int ReadfifoDescripter;
    int WritefifoDescripter;
    int ShowfifoDescripter;
    int clientPid;
    int proc,count=0;
    pid_t pid,pid2;
    struct timeval tStart;
    struct timeval tEnd;
    struct sigaction act2;



    clientPid = (int)getpid();
    information.inComPid=clientPid;
    sprintf(ReadFifo , "read_" );
    sprintf(WriteFifo , "write_");

    if(argc !=2){
        fprintf(stderr,"Usage:./SeeWhat <mpipename>\n");
        exit(EXIT_FAILURE);
    }
    
    /*------------------------mainfifo-------------------------------------*/
    
    fd = open(argv[1], O_RDWR);
    
    if (fd == -1) {
        fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
        (long)getpid(), argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    read(fd,&mainPid,sizeof(int));
    mainServer=mainPid;
    write(fd , &mainPid, sizeof(int));
    
    close(fd);

    /*--------------------------------------------------------------------*/
    
    /*-----------------signal gonderme request---------------------------*/
    value.sival_int=getpid();
    /*printf("pid = %d\n",getpid());*/
    if(sigqueue(mainPid,SIGRTMIN,value)==-1){
        perror("Failed to send the signal");
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
    /*-------------------------------------------------------------------*/
    sprintf(pidFifoName,"%d",(int)getpid());
    strcat(ReadFifo ,pidFifoName);
    strcat(WriteFifo,pidFifoName);
    strcpy(rClient,ReadFifo);
    strcpy(wClient,WriteFifo);

    /*------------------------pidfifo-------------------------------------*/
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


    if (mkfifo(SHOWRESULTS_FIFO, FIFO_PERMS) == -1) { 
        if (errno != EEXIST) {
            fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
            (long)getpid(), SHOWRESULTS_FIFO, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }  

    ReadfifoDescripter = open(ReadFifo, O_RDWR) ;
    WritefifoDescripter = open(WriteFifo, O_WRONLY) ;
/**************************************************************/
    ShowfifoDescripter = open(SHOWRESULTS_FIFO, O_RDWR) ;
    if (ShowfifoDescripter == -1) {
        fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
        (long)getpid(), SHOWRESULTS_FIFO, strerror(errno));
        exit(EXIT_FAILURE);
    }
/***************************************************************/
    if (ReadfifoDescripter == -1) {
        fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
        (long)getpid(), ReadFifo, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (WritefifoDescripter == -1) {
        fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
        (long)getpid(), WriteFifo, strerror(errno));
        exit(EXIT_FAILURE);
    }
 
    while(1)
    {
        pipe(fdp);
        read(ReadfifoDescripter, &matrix , sizeof(struct matrixInf));

        sprintf(seeWhatLog,"log/SeeWhat_%d_%d.log",getpid(),count);
        /*printf("%s\n",seeWhatLog );*/
        logPtr=fopen(seeWhatLog,"a");
        shiftedFourDivision(matrix.matrixArr,aMat,bMat,cMat,dMat,matrix.size);
             

        pid=fork();
        if(pid==0){
            close(fdp[0]);
            gettimeofday(&tStart,NULL);
            convolutionMatrix2D(aMat,matrix.size/2);
            convolutionMatrix2D(bMat,matrix.size/2);
            convolutionMatrix2D(cMat,matrix.size/2);
            convolutionMatrix2D(dMat,matrix.size/2);
      
                for(i = 0; i < matrix.size/2 ; i++){
                    for(j = 0; j < matrix.size/2; j++)
                         shiftedConvolution[i][j]=aMat[i][j];
                }
                for(i = 0; i < matrix.size/2 ; i++){
                    for(j = matrix.size/2, k = 0; j < matrix.size; j++,k++)
                        shiftedConvolution[i][j]=bMat[i][k];
                }
                for(i = matrix.size/2, k=0; i < matrix.size ; i++,k++){
                    for(j = 0; j < matrix.size/2; j++)
                        shiftedConvolution[i][j]=cMat[k][j];
                }
                for(i = matrix.size/2, k = 0; i < matrix.size ; i++,k++){
                    for(j = matrix.size/2, l = 0; j < matrix.size; j++,l++)
                        shiftedConvolution[i][j]=dMat[k][l];
                }
                
                information.result2 = matrix.orjDetValue - determinant(shiftedConvolution,matrix.size);
                gettimeofday(&tEnd,NULL);
                information.timeElapsed2 = 1000*(tEnd.tv_sec - tStart.tv_sec) + (tEnd.tv_usec - tStart.tv_usec)/1000.0;
                write(fdp[1],&information.timeElapsed2,sizeof(long));
                write(fdp[1],&information.result2,sizeof(double));
                close(fdp[1]);

                fprintf(logPtr,"ShiftedConv = [");
                for(i=0; i<matrix.size;i++){
                    for(j=0; j<matrix.size;j++){
                        fprintf(logPtr,"%f", shiftedConvolution[i][j]);
                        if(j!=matrix.size-1)
                            fprintf(logPtr," ");
                    }
                    if(i!=matrix.size-1)
                        fprintf(logPtr,"; ");
                }
                fprintf(logPtr,"]\n");

            exit(1);
        }
        if(pid>0){
            pid2=fork();
            if(pid2==0){
                gettimeofday(&tStart,NULL);
                cofactor(aMat, matrix.size/2);
                cofactor(bMat, matrix.size/2);
                cofactor(cMat, matrix.size/2);
                cofactor(dMat, matrix.size/2);

                for(i = 0; i < matrix.size/2 ; i++){
                    for(j = 0; j < matrix.size/2; j++)
                         shiftedInverse[i][j]=aMat[i][j];
                }
                for(i = 0; i < matrix.size/2 ; i++){
                    for(j = matrix.size/2, k = 0; j < matrix.size; j++,k++)
                        shiftedInverse[i][j]=bMat[i][k];
                }
                for(i = matrix.size/2, k=0; i < matrix.size ; i++,k++){
                    for(j = 0; j < matrix.size/2; j++)
                        shiftedInverse[i][j]=cMat[k][j];
                }
                for(i = matrix.size/2, k = 0; i < matrix.size ; i++,k++){
                    for(j = matrix.size/2, l = 0; j < matrix.size; j++,l++)
                        shiftedInverse[i][j]=dMat[k][l];
                }
                close(fdp[0]);
                information.result1 = matrix.orjDetValue - determinant(shiftedInverse,matrix.size);
                
                gettimeofday(&tEnd,NULL);
                information.timeElapsed1 = 1000*(tEnd.tv_sec - tStart.tv_sec) + (tEnd.tv_usec - tStart.tv_usec)/1000.0;
                write(fdp[1],&information.timeElapsed1,sizeof(long));
                write(fdp[1],&information.result1,sizeof(double));
                close(fdp[1]);

                fprintf(logPtr,"ShiftedInv = [");
                for(i=0; i<matrix.size;i++){
                    for(j=0; j<matrix.size;j++){
                        fprintf(logPtr,"%f", shiftedInverse[i][j]);
                        if(j!=matrix.size-1)
                            fprintf(logPtr," ");
                    }
                    if(i!=matrix.size-1)
                        fprintf(logPtr,"; ");
                }
                fprintf(logPtr,"]\n");
                exit(1);
                if(pid2>0)
                    wait(NULL);
            }
            wait(NULL);
            fprintf(logPtr,"Org = [");
                for(i=0; i<matrix.size;i++){
                    for(j=0; j<matrix.size;j++){
                        fprintf(logPtr,"%f", matrix.matrixArr[i][j]);
                        if(j!=matrix.size-1)
                            fprintf(logPtr," ");
                    }
                    if(i!=matrix.size-1)
                        fprintf(logPtr,"; ");
                }
                fprintf(logPtr,"]\n");
            fclose(logPtr);
        }

        write(WritefifoDescripter,&(clientPid) , sizeof(int));
        count++;
        close(fdp[1]);
        read(fdp[0],&information.timeElapsed2,sizeof(long));
        read(fdp[0],&information.result2,sizeof(double));
        read(fdp[0],&information.timeElapsed1,sizeof(long));
        read(fdp[0],&information.result1,sizeof(double));
        close(fdp[0]);
        
    
        write(ShowfifoDescripter,&information,sizeof(struct incomingInf));
    }

    return (EXIT_SUCCESS);
}
/*Matris Determinantinin hesaplanmasi*/
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
/* Matrisin Tersi icin Cofactor Hesabi (*/
void cofactor(double num[MATRIX_SIZE][MATRIX_SIZE], int f)
{
    double b[MATRIX_SIZE][MATRIX_SIZE], fac[MATRIX_SIZE][MATRIX_SIZE];
    int k, l, m, n, i, j;
    
    if(f==1){
        num[0][0]=1.0/num[0][0];
    }
    else{
        for (l = 0;l < f; l++)
        {
            for (k = 0;k < f; k++)
            {
                m = 0;
                n = 0;
                for (i = 0;i < f; i++)
                {
                    for (j = 0;j < f; j++)
                    {
                        if (i != l && j != k)
                        {
                            b[m][n] = num[i][j];
                            
                            if (n < (f - 2))
                                n++;
                            else
                            {
                               n = 0;
                               m++;
                            }
                        }
                    }
                }
                fac[l][k] = pow(-1, l + k) * determinant(b, f - 1);
            }
        }
        transpose(num, fac, f);
    }
}
/*Finding transpose of matrix*/ 
void transpose(double num[MATRIX_SIZE][MATRIX_SIZE], double fac[MATRIX_SIZE][MATRIX_SIZE], int r)
{
    int i, j;
    double b[MATRIX_SIZE][MATRIX_SIZE], inverse[MATRIX_SIZE][MATRIX_SIZE], d;
    
    for (i = 0;i < r; i++){
        for (j = 0;j < r; j++)
            b[i][j] = fac[j][i];
    }
    d = determinant(num, r);
    for (i = 0;i < r; i++){
        for (j = 0;j < r; j++)
            num[i][j] = b[i][j] / d;
        
    }

}

/* 2D Convolution Matrix elde etme (internetten alinti)*/
void convolutionMatrix2D(double matrix[MATRIX_SIZE][MATRIX_SIZE],int k){
    
    int i,j,m,mm,n,nn,ii,jj,kCenterX,kCenterY,kCols,kRows;
    double convolMatrix[MATRIX_SIZE][MATRIX_SIZE],kernel[3][3]={{0,0,0},{0,1,0},{0,0,0}};
    kCols=3;
    kRows=3;

    for(i = 0;i < k; i++){
        for(j = 0;j < k;j++)
            convolMatrix[i][j]=0; 
    }
    
    kCenterX = kCols / 2;
    kCenterY = kRows / 2;

    for(i=0; i < k; ++i)              
    {
        for(j=0; j < k; ++j)          
        {
            for(m=0; m < kRows; ++m)     
            {
                mm = kRows - 1 - m;      
                for(n=0; n < kCols; ++n) 
                {
                    nn = kCols - 1 - n;  
                    ii = i + (m - kCenterY);
                    jj = j + (n - kCenterX);
                    if( ii >= 0 && ii < k && jj >= 0 && jj < k )
                        convolMatrix[i][j] += matrix[ii][jj] * kernel[mm][nn];;
                    
                        
                }
            }  
        }
    }
  
}

/* Shifted ve dort parcaya bolme*/
void shiftedFourDivision(double mainMatrix[MATRIX_SIZE][MATRIX_SIZE],
                         double firstMatrix[MATRIX_SIZE][MATRIX_SIZE],
                         double secondMatrix[MATRIX_SIZE][MATRIX_SIZE],
                         double thirdMatrix[MATRIX_SIZE][MATRIX_SIZE],
                         double fourthMatrix[MATRIX_SIZE][MATRIX_SIZE],int size){
    
    int i,j,k,l;
    for(i = 0; i < size/2 ; i++){
        for(j = 0; j < size/2; j++)
            firstMatrix[i][j]=mainMatrix[i][j];
    }
    for(i = 0; i < size/2 ; i++){
        for(j = size/2, k = 0; j < size; j++,k++)
            secondMatrix[i][k]=mainMatrix[i][j];
    }
    for(i = size/2, k=0; i < size ; i++,k++){
        for(j = 0; j < size/2; j++)
            thirdMatrix[k][j]=mainMatrix[i][j];
    }
    for(i = size/2, k = 0; i < size ; i++,k++){
        for(j = size/2, l = 0; j < size; j++,l++)
            fourthMatrix[k][l]=mainMatrix[i][j];
    }
    
}
