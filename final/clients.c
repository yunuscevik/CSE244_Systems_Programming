
/* 
 * File:   main.c
 * Author: asus-ubuntu
 *
 * Created on 25 Mayıs 2017 Perşembe, 21:21
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <sys/socket.h>    
#include <arpa/inet.h> 
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <wait.h>
#define SIZE 50
#define THOUSAND 1000.0L
#define LOG_DIRECTORY "log"
#define LOGFILE "log/clients.log"

/*
 * 
 */
sem_t sem , sem2;
int isMain=1;
struct timeval timeEnd,timeStart;
double *times;
int count=0;
int threadSize,flag=0;
struct matrixInf{
    int col;
    int row;
	int clientPid;
};
struct  sendMatrixInf
{
	int col;
	int row;
        double A[SIZE][SIZE];
        double B[SIZE][SIZE];
        double X[SIZE][SIZE];
        double e;
        double time;
        
};
struct  sendMatrixInf sendMatrix;
struct matrixInf matrix;

static void closeCTRLC(int signo){
    int i;
    char *ctrlC="CTRL + C pressed signal no =>>> SIGINT(#2)";
    if(isMain==1){
        if(signo==SIGINT){
            
            while(wait(NULL)!=-1);
            fprintf(stderr, "\n%s\n",ctrlC);
            
            pthread_detach(pthread_self());
        }
    }
    
    exit(1);
    
}

void *sendToSocketWithTherad(void * data){
    
    int socket, i,j;
    FILE *clientPtr;
	char clientlog[255];
	
	while(flag!=threadSize);
	sprintf(clientlog,"log/client_%ld_%d.log",(long)pthread_self(),count+1);
 	clientPtr=fopen(clientlog,"a");
    socket=*((int*)data);
	gettimeofday(&timeStart,NULL);
    sem_wait(&sem);
        write(socket,&matrix,sizeof(matrix));
    sem_post(&sem);
    
    
    sem_wait(&sem2);
		
        if(recv(socket,&sendMatrix,sizeof(sendMatrix),MSG_WAITALL)==-1){
                    perror("read");
            }


            printf("\n");
            
			fprintf(clientPtr,"row=> %d\ncol=> %d\ne=>> %.4f \n",sendMatrix.row,sendMatrix.col,sendMatrix.e);
			fprintf(clientPtr,"\nA matrix\n");
            for (i = 0;i < sendMatrix.row; i++){

                for (j = 0;j <sendMatrix.col; j++)
                    fprintf(clientPtr,"%.4f\t",sendMatrix.A[i][j]);
                
                fprintf(clientPtr,"\n");

            }
			fprintf(clientPtr,"\nB Vector\n");
            for (j = 0;j < sendMatrix.row; j++)
                    fprintf(clientPtr,"%.4f\t",sendMatrix.B[j][0]);
            
            fprintf(clientPtr,"\n"); 
			fprintf(clientPtr,"\nC Vector\n");  
            for (j = 0;j <sendMatrix.col; j++)
            	fprintf(clientPtr,"%.4f\t",sendMatrix.X[j][0]);
            
            fprintf(clientPtr,"\n");

        gettimeofday(&timeEnd,NULL);
        times[count]=THOUSAND * (timeEnd.tv_sec-timeStart.tv_sec) + (timeEnd.tv_usec - timeStart.tv_usec)/THOUSAND;
        count++;
		fclose(clientPtr);
    sem_post(&sem2);
		
 	
}
int main(int argc, char** argv) {
    FILE *clientsPtr;
    pthread_t *tid;
    int portNo = 0;
    int col=0, row=0;
    int i;
    int *sockfd;
    double sum=0.0,avg=0.0,stdDeviation=0.0;
    struct sigaction act;
    struct sockaddr_in serv_addr;
    
    if(argc != 5){
        fprintf(stderr,"./clients <#of columns of A, m> <#of rows of A, p> <#of clients, q> <# of port>");
        exit(EXIT_FAILURE);
    }

    col=atoi(argv[1]);
    row=atoi(argv[2]);
    threadSize=atoi(argv[3]);
    portNo=atoi(argv[4]);
    times=(double *)malloc(threadSize * sizeof(double));
    sockfd=(int *)malloc(threadSize * sizeof(int));
    tid=(pthread_t*)malloc( threadSize * sizeof(pthread_t));
    sem_init(&sem,0,1);
    sem_init(&sem2,0,1);     
    matrix.col=col;
    matrix.row=row;
    matrix.clientPid=(int)getpid();
    for(i=0; i<threadSize; i++){
        /*************************soket ve connect olusturma************************/
        if ((sockfd[i] = socket(AF_INET , SOCK_STREAM , 0)) < 0){
            fprintf(stderr,"Could not create socket\n");
            exit(EXIT_FAILURE);
        }
        
        memset(&serv_addr, '0', sizeof(serv_addr));
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(portNo);
		act.sa_flags = 0;
		act.sa_handler = closeCTRLC;
		if ((sigemptyset(&act.sa_mask) == -1) ||
		    (sigaction(SIGINT, &act, NULL) == -1)){
		    perror("Failed to set up SIGINT signal");
		    return 1;
		}
		
        if( connect(sockfd[i], (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
           fprintf(stderr,"Connect Failed \n");
           exit(EXIT_FAILURE);
        }
        /***************************************************************************/

        /**********************threadler olusturuluyor******************************/
		flag++;
        if(pthread_create(&tid[i],NULL,sendToSocketWithTherad,&sockfd[i])<0){
            perror("could not create thread");
            exit(EXIT_FAILURE);
        }
        /**************************************************************************/
        
    }
   	
    for(i=0; i<threadSize; i++){
        pthread_join(tid[i],NULL);
        close(sockfd[i]);
    }
    clientsPtr=fopen(LOGFILE,"a");
    if(clientsPtr==NULL){
        perror("fopen error");
        exit(EXIT_FAILURE);
    }
    for(i=0; i<threadSize; i++)
    	sum += times[i];
    avg=sum/threadSize;
	fprintf(clientsPtr,"Clients pid == > %d\n",getpid());
    fprintf(clientsPtr,"Average connection time == > %f ms\n",avg);
    for(i=0; i<threadSize; i++)
        stdDeviation += (pow((times[i]-avg),2)) / threadSize;
	stdDeviation=sqrt(stdDeviation);
    fprintf(clientsPtr,"Standard Deviation == > %f\n\n",stdDeviation);
    fclose(clientsPtr);
    free(times);
    free(sockfd);
    free(tid);
    sem_destroy(&sem);
    
    return (EXIT_SUCCESS);
}

