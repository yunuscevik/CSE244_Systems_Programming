#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>    
#include <pthread.h> 
#include <time.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <math.h>
#include <wait.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "matrixMethods.h"

#define LOG_DIRECTORY "log"

#define PERM S_IRUSR | S_IWUSR


int sid,sid2;
int i=0;
int flag=1;
int isMain=1;
int clientPid[2000];
int tcount=0;
sem_t semWithChild1,semWithChild2;
void verify(double A[][SIZE],double B[][SIZE],double X[][SIZE],int m,int p,int sendSockFd);
void randomMatrix(double randMatrix[][SIZE], double bMatrix[SIZE][SIZE], int col, int row);
int detachandremove(int shmid, void *shmaddr);
struct  rcvMatrixInf
{
	int col;
	int row;
        int clientPid;
};
struct  rcvMatrixInf rcvInf;

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
struct  sendMatrixInf sendInf;

struct  sharedMemControl
{
	pthread_mutex_t control;
	pthread_mutex_t control2;
};
struct  sharedMemControl stmc;

struct  randomMatrixInf
{
    double stAmatrix[SIZE][SIZE];
    double stBmatrix[SIZE][SIZE];
    int m;
    int p;
    int flag;
};
struct  randomMatrixInf *rndM;

struct  sharedMemoryBlock
{
    double stAmatrix[SIZE][SIZE];
    double stBmatrix[SIZE][SIZE];
    double stXmatrix[SIZE][SIZE];
    int m;
    int p;
    int flag;
};
struct  sharedMemoryBlock *stmb;


void *SVDThreadFonk(void *data){
    struct sharedMemControl stmC=(*(struct sharedMemControl*)data);
    int i,j;
    double A[SIZE][SIZE],B[SIZE][SIZE],X[SIZE][SIZE];
    pthread_mutex_lock(&stmC.control);
    for (i = 0;i < rndM->m; i++){
        B[i][0]=rndM->stBmatrix[i][0];
        for (j = 0;j < rndM->p; j++){
            A[i][j]=rndM->stAmatrix[i][j];
        }
    }
    pthread_mutex_unlock(&stmC.control);
    
    pseudoInverse(A,B,X,rndM->m,rndM->p);
        
        
    pthread_mutex_lock(&stmC.control2);
    stmb->m=rndM->m;
    stmb->p=rndM->p;
    for (i = 0;i < stmb->m; i++){
        stmb->stBmatrix[i][0]=B[i][0];
        for (j = 0;j < stmb->p; j++){
            stmb->stAmatrix[i][j]=A[i][j];
        } 
        
    }
    for (j = 0;j < stmb->p; j++){
        stmb->stXmatrix[j][0]=X[j][0];
    }
    stmb->flag=1;
    pthread_mutex_unlock(&stmC.control2);
    
    
    
}
void *QRThreadFonkGauss(void *data){
    struct sharedMemControl stmC=(*(struct sharedMemControl*)data);
    int i,j;
    double A[SIZE][SIZE],B[SIZE][SIZE],X[SIZE][SIZE];
    double x1D[SIZE],b1D[SIZE],aT[SIZE][SIZE],aTb[SIZE][SIZE],aTa[SIZE][SIZE];
    pthread_mutex_lock(&stmC.control);
    for (i = 0;i < rndM->m; i++){
        B[i][0]=rndM->stBmatrix[i][0];
        for (j = 0;j < rndM->p; j++){
            A[i][j]=rndM->stAmatrix[i][j];
        }
    }
    pthread_mutex_unlock(&stmC.control);
    
    transpose(A,aT,rndM->m,rndM->p);
    matrixMult(aTa,aT,A,rndM->p,rndM->m,rndM->p);
    matrixMult(aTb,aT,B,rndM->p,rndM->m,1);
    
    for (i = 0;i < rndM->p; i++)
        b1D[i]=B[i][0];
    
    gaussianElimination(aTa,b1D,x1D,rndM->p);
    
    for (i = 0;i < rndM->p; i++)
        X[i][0]=x1D[i];
    
    printf("\nGAUSELIMINATION WITH SCALED PARTIAL PIVOT\n");
    for (i = 0;i < rndM->m; i++){
        for (j = 0;j < rndM->p; j++){
            printf("%f \t", A[i][j]);  
        } 
        printf("\n");
    }
    printf("\n");
    
    for (j = 0;j < rndM->p; j++){
        printf("%f \t", B[j][0]);  
    } 
    printf("\n");
    
    
    
    for (j = 0;j < rndM->p; j++){
        printf("%f\t",X[j][0]);
    }
    
    
    pthread_mutex_lock(&stmC.control2);
    stmb->m=rndM->m;
    stmb->p=rndM->p;
    for (i = 0;i < stmb->m; i++){
        stmb->stBmatrix[i][0]=B[i][0];
        for (j = 0;j < stmb->p; j++){
            stmb->stAmatrix[i][j]=A[i][j];
        } 
        
    }
    for (j = 0;j < stmb->p; j++){
        stmb->stXmatrix[j][0]=X[j][0];
    }
    stmb->flag=1;
    pthread_mutex_unlock(&stmC.control2);
    
}
void *PSInversethreadFonk(void *data){
    struct sharedMemControl stmC=(*(struct sharedMemControl*)data);
    int i,j;
    double A[SIZE][SIZE],B[SIZE][SIZE],X[SIZE][SIZE];
    pthread_mutex_lock(&stmC.control);
    for (i = 0;i < rndM->m; i++){
        B[i][0]=rndM->stBmatrix[i][0];
        for (j = 0;j < rndM->p; j++){
            A[i][j]=rndM->stAmatrix[i][j];
        }
    }
    pthread_mutex_unlock(&stmC.control);
    
    pseudoInverse(A,B,X,rndM->m,rndM->p);
    printf("\nPSUEDO INVERSE\n");
    for (i = 0;i < rndM->m; i++){
            for (j = 0;j < rndM->p; j++){
                printf("%f \t", A[i][j]);  
            } 
            printf("\n");
        }
        printf("\n");
       
        for (j = 0;j < rndM->m; j++){
            printf("%f \t", B[j][0]);  
        } 
        printf("\n");
        
    
    
    for (j = 0;j < rndM->p; j++){
        printf("%f\t",X[j][0]);
    }
    
    
    
    pthread_mutex_lock(&stmC.control2);
    stmb->m=rndM->m;
    stmb->p=rndM->p;
    for (i = 0;i < stmb->m; i++){
        stmb->stBmatrix[i][0]=B[i][0];
        for (j = 0;j < stmb->p; j++){
            stmb->stAmatrix[i][j]=A[i][j];
        } 
        
    }
    for (j = 0;j < stmb->p; j++){
        stmb->stXmatrix[j][0]=X[j][0];
    }
    stmb->flag=1;
    pthread_mutex_unlock(&stmC.control2);
    
    
    
}




void *serverThreadsFunction(void *data)
{

    int sock = *(int*)data;
    int readVal;
    pthread_t pid[3];
    
    
    int i,j;
    pthread_mutex_t mControl = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mControl2 = PTHREAD_MUTEX_INITIALIZER;
    
    stmc.control=mControl;
    stmc.control2=mControl2;
    if( (readVal = recv(sock , &rcvInf , sizeof(rcvInf) , 0)) > 0 ){
        printf("Server receive from Client  row=> %d - col=> %d \n",rcvInf.row,rcvInf.col );
       
        
    }
    
        if(clientPid[tcount]!=rcvInf.clientPid){
            clientPid[tcount]=rcvInf.clientPid;
            tcount++;
        }
    
         
    if ((sid = shmget(IPC_PRIVATE,sizeof(struct randomMatrixInf), IPC_CREAT | PERM)) == -1) {
        perror("Failed to create shared memory segment");
        exit(EXIT_FAILURE);
    }
    if ((rndM = (struct  randomMatrixInf *)shmat(sid, NULL, 0)) == (void *)-1) {
        perror("Failed to attach shared memory segment");
        if (shmctl(sid, IPC_RMID, NULL) == -1)
            perror("Failed to remove memory segment");
        exit(EXIT_FAILURE);
    }
    
    
    
    if ((sid2 = shmget(IPC_PRIVATE,sizeof(struct sharedMemoryBlock), IPC_CREAT | PERM)) == -1) {
        perror("Failed to create shared memory segment");
        exit(EXIT_FAILURE);
    }
    if ((stmb = (struct  sharedMemoryBlock *)shmat(sid2, NULL, 0)) == (void *)-1) {
        perror("Failed to attach shared memory segment");
        if (shmctl(sid2, IPC_RMID, NULL) == -1)
            perror("Failed to remove memory segment");
        exit(EXIT_FAILURE);
    }
    
    
    
    
    for(i=0 ; i<3 ; i++)
        pid[i]= -2;

    for(i=0 ; i < 3 ; i++){
        if((pid[0] != 0 && pid[1] != 0 && pid[2] != 0)){
            if((pid[i]=fork())<0){
                perror("Unsuccesful fork() ");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    
    if(pid[0]==0){
        isMain=0;
        randomMatrix(rndM->stAmatrix,rndM->stBmatrix,rcvInf.col,rcvInf.row);
        rndM->m=rcvInf.row;
        rndM->p=rcvInf.col;
        rndM->flag=1;
        exit(EXIT_SUCCESS);  
    }
    if(pid[1]==0){
        pthread_t tid1,tid2,tid3;
        int error;
        isMain=0;
        while(rndM->flag != 1);
        if( pthread_create( &tid1 , NULL ,  SVDThreadFonk , (void*) &stmc) < 0){
            perror("could not create thread");
            exit(EXIT_FAILURE);
        }
        if( pthread_create( &tid2 , NULL ,  QRThreadFonkGauss , (void*) &stmc) < 0){
            perror("could not create thread");
            exit(EXIT_FAILURE);
        }
        if( pthread_create( &tid3 , NULL ,  PSInversethreadFonk , (void*) &stmc) < 0){
            perror("could not create thread");
            exit(EXIT_FAILURE);
        }
        
        if(error = pthread_join(tid1, NULL)==-1){
            fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
        }
        if(error = pthread_join(tid2, NULL)==-1){
            fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
        }
        if(error = pthread_join(tid3, NULL)==-1){
            fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
        }
        exit(EXIT_SUCCESS);  
    }
    if(pid[2]==0){
        int i;
        isMain=0;
        while(stmb->flag != 1);
            verify(stmb->stAmatrix,stmb->stBmatrix,stmb->stXmatrix,stmb->m,stmb->p,sock);
            stmb->flag=0;
        exit(EXIT_SUCCESS);   
        
    }
    
    while((wait(NULL))!=-1);
    
        
        pthread_detach(pthread_self());
    
    
    pthread_mutex_destroy(&mControl);
    pthread_mutex_destroy(&mControl2);
            
    close(sock);
    free(data);
}




static void closeCTRLC(int signo){
    int i;
    char *ctrlC="CTRL + C pressed signal no =>>> SIGINT(#2)";
    if(isMain==1){
        if(signo==SIGINT){
            for(i=0; i<tcount;i++)
                kill(clientPid[i],SIGINT);
            while(wait(NULL)!=-1);
            fprintf(stderr, "\n%s\n",ctrlC);
            shmdt(rndM);
            shmdt(stmb);
             
           
            /*if(detachandremove(sid, rndM) == -1) {
                perror("Failed to destroy shared memory segment");
                exit(EXIT_FAILURE);
            }
            if(detachandremove(sid2, stmb) == -1) {
                perror("Failed to destroy shared memory segment");
                exit(EXIT_FAILURE);
            }*/
            pthread_detach(pthread_self());
        }
    }
    
    exit(1);
    
}

int main(int argc , char *argv[])
{
    
    struct sigaction signal;
    sigset_t mask;
    
    
    
    int sockFd , clientSockAccept ,sizeOf , *newSocketPtr;
    struct sockaddr_in server , client;
    pthread_t tid;
    char path[PATH_MAX];
    struct sigaction act;
    if(argc !=2){
        fprintf(stderr, "./server <port #, id>\n" );
        exit(EXIT_FAILURE);
    }
    /*******log klasoru*******/
    getcwd(path,PATH_MAX);
    strcat(path,"/");
    strcat(path,LOG_DIRECTORY);
    system("rm -rf log/");
    mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    /************************************************/
    sem_init(&semWithChild1,0,1);
    sem_init(&semWithChild2,0,1);
    

    act.sa_flags = 0;
    act.sa_handler = closeCTRLC;
    if ((sigemptyset(&act.sa_mask) == -1) ||
        (sigaction(SIGINT, &act, NULL) == -1)){
        perror("Failed to set up SIGINT signal");
        return 1;
    }
    sockFd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockFd == -1){
        printf("Could not create socket");
        exit(EXIT_FAILURE);
    }
    puts("Socket created");
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( atoi(argv[1]) );
     
    
    if( bind(sockFd,(struct sockaddr *)&server , sizeof(server)) < 0){
        perror("bind failed. Error");
        exit(EXIT_FAILURE);
    }
    listen(sockFd , 1000);
    puts("Waiting for incoming connections...");
	sizeOf=sizeof(struct sockaddr_in);
    while( (clientSockAccept = accept(sockFd, (struct sockaddr *)&client,&sizeOf ))){
        puts("Connection accepted");

        newSocketPtr = malloc(sizeof(int));
        *newSocketPtr = clientSockAccept;
         
        if( pthread_create( &tid , NULL ,  serverThreadsFunction , (void*) newSocketPtr) < 0){
            perror("could not create thread");
            exit(EXIT_FAILURE);
        }
    }
    
    if (clientSockAccept < 0){
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
     
    return 0;
}



void randomMatrix(double randMatrix[][SIZE], double bMatrix[SIZE][SIZE], int col, int row){
    struct timespec timers;
    clock_gettime(CLOCK_MONOTONIC, &timers);
    srand((time_t)timers.tv_nsec);
    int i,j;
    for (i = 0; i < row; i++){
        bMatrix[i][0]=rand()%21+(-10);
    }
    
    for (i = 0;i < row; i++){
        for (j = 0;j < col; j++){
            randMatrix[i][j]=rand()%21+(-10);  
        } 
    }
    
}
int detachandremove(int shmid, void *shmaddr) {
   int error = 0; 

   if (shmdt(shmaddr) == -1)
      error = errno;
   if ((shmctl(shmid, IPC_RMID, NULL) == -1) && !error)
      error = errno;
   if (!error)
      return 0;
   errno = error;
   return -1;
}




void verify(double A[][SIZE],double B[][SIZE],double X[][SIZE],int m,int p,int sendSockFd){
    double e[SIZE][SIZE],eT[SIZE][SIZE],eTe[SIZE][SIZE],Ax[SIZE][SIZE],Norme=0.0;
    int i,j;
    FILE *sLog;
    char serverlog[255];
    matrixMult(Ax,A,X,m,p,1);
    for(i=0; i < m;i++)
        e[i][0]=Ax[i][0]-B[i][0];
    transpose(e,eT,m,p);
    matrixMult(eTe,eT,e,1,m,1);
    Norme=sqrt(eTe[0][0]);
    sprintf(serverlog,"log/server_%ld.log",(long)pthread_self());
    sLog=fopen(serverlog,"a");
    if(sLog==NULL){
        perror("fopen error");
        exit(EXIT_FAILURE);
    }
    sendInf.row=m;
    sendInf.col=p;
    sendInf.e=Norme;
    fprintf(sLog,"A matrix\n");
    for (i = 0;i < m; i++){
        for (j = 0;j < p; j++){
            sendInf.A[i][j]=A[i][j];
            fprintf(sLog,"%.4f\t",A[i][j]);
        } 
        fprintf(sLog,"\n");
    }
    fprintf(sLog,"\n");
    fprintf(sLog,"B matrix\n");
    for (i = 0;i < m; i++){
        sendInf.B[i][0]=B[i][0];
        fprintf(sLog,"%.4f\t",B[i][0]);
    }
    fprintf(sLog,"\n");
    fprintf(sLog,"C matrix\n");
    for (j = 0;j < p; j++){
        sendInf.X[j][0]=X[j][0];
        fprintf(sLog,"%.4f\t",X[j][0]);
    }
    fprintf(sLog,"\n\n");
    fclose(sLog);
    send(sendSockFd,&sendInf,sizeof(sendInf),0);
    stmb->flag=1;
    
    
    
}


