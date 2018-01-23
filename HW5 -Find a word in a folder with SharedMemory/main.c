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
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#define SEEK_CUR 1
#define LOGFILE "log.txt"
#define FILEVALUE 8
#define DIRECTORYVALUE 4
#define ARGCVALUE 3
#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define PERM (S_IRUSR | S_IWUSR)
#define PATH_MAX 4096
#define PIPE_BUF 4096
#define STR_SIZE 255
#define THRD_SIZE 255
#define SEARCH_DIRECTORIES "searchDirectories.txt"
#define SEARCH_FILES "searchFiles.txt"
#define TOTAL_THREADS "totalThreads.txt"
#define SEARCH_LINES "searchLines.txt"
#define MAX_THREAD_NUM "maxThreadsNum.txt"
#define THREAD_IDS "threadIds.txt"
#define SHARED_MEM_SIZES "shmSizes.txt"
#define THOUSAND 1000.0L


void readFileAndFindStr(char *srcStr,char *filePath, char *file,int *total);
void isDirectoryOrFile(char *srcStr, char *dirName,int mid);
void printLog(long pid,long tid,char *fileName,int row,int col,char *srcString);
void totalNumberOfMatchUp(int count,char *srcStr,double timedif,char*exitCondition);
int detachandremove(int shmid, void *shmaddr);

sem_t sem;
FILE *threadIdPtr;
int isMain=1;
int totCount=0;
int *sharedTotal=0;
int id;
int msid;
int msqid;
char searchStr[STR_SIZE];
double difT=0.0;
struct timeval timeEnd,timeStart;
static volatile sig_atomic_t doneflag=0;

struct msgBuf{
    long mType;
    int mValue;
};
struct msgBuf msRcv;

struct forThreadFunc{
    int x;
    int *num;
};
struct forThreadFunc thData;

struct searchStrArg{
    char str[STR_SIZE];
    char path[PATH_MAX];
    char fileName[STR_SIZE];
};
struct searchStrArg fonkArgs[THRD_SIZE];


void *searchStrWithThread(void *data){
    struct forThreadFunc info;
    info=*((struct forThreadFunc *)data);
    if(!doneflag){
        threadIdPtr=fopen(THREAD_IDS,"a");
        fprintf(threadIdPtr,"%ld\n",(long)pthread_self());
        fclose(threadIdPtr);
        readFileAndFindStr(fonkArgs[info.x].str,fonkArgs[info.x].path,fonkArgs[info.x].fileName, info.num);
    }
    sem_post(&sem);/* semaphore dan cikis kismi*/
    
}
static void closeCTRLC(int signo){
    char *ctrlC="CTRL + C pressed signal no =>>> SIGINT(#2)";
    if(isMain==1){
        if(signo==SIGINT){
            doneflag=1;
            
            while(wait(NULL)!=-1);
            
            fprintf(stderr, "\n%s\n",ctrlC);
            gettimeofday(&timeEnd,NULL);
            difT=THOUSAND * (timeEnd.tv_sec-timeStart.tv_sec) + (timeEnd.tv_usec - timeStart.tv_usec)/THOUSAND;
            totalNumberOfMatchUp(totCount,searchStr,difT,ctrlC);
           
        }
    }
    
    shmdt(sharedTotal);
    shmctl(id, IPC_RMID, NULL);
    msgctl(msqid,IPC_RMID,NULL);
    msgctl(msid,IPC_RMID,NULL);
    exit(1);
    
}
int main(int argc, char** argv) {
    
    pid_t pid;
    char path[PATH_MAX];
    double timeDifVal=0.0;
    char *exCon="Succesful";
    struct sigaction act;

    gettimeofday(&timeStart,NULL);
    if(argc !=ARGCVALUE)
    {
        fprintf(stderr, "Usage: ./grepSh 'string'  <directoryName> \n");
        exit(EXIT_FAILURE);
    }
    remove(LOGFILE);
    remove(THREAD_IDS);
    remove(SEARCH_DIRECTORIES);
    remove(SEARCH_FILES);
    remove(TOTAL_THREADS);
    remove(SEARCH_LINES);
    remove(MAX_THREAD_NUM);
    remove(SHARED_MEM_SIZES);
    
    getcwd(path,PATH_MAX);
    strcat(path,"/");
    strcat(path,argv[2]);
    strcpy(searchStr,argv[1]);
    
    

    if ((msid = msgget(IPC_PRIVATE, PERM | IPC_CREAT )) == -1)
        perror("Failed to create new private message queue");

    if(!doneflag){
        pid=fork();
        if(pid==0){
            
            if(!doneflag){
                fprintf(stdout,"Number of cascade threads created: %s/",argv[2]);
            }
            act.sa_flags = 0;
            act.sa_handler = closeCTRLC;
            if ((sigemptyset(&act.sa_mask) == -1) ||
                (sigaction(SIGINT, &act, NULL) == -1)){
                perror("Failed to set up SIGINT signal");
                return 1;
            }
            isDirectoryOrFile(argv[1],path,msid);
            exit(EXIT_SUCCESS);
        }
        else if(pid>0){

            if(msgrcv(msid,&msRcv,sizeof(struct msgBuf),msRcv.mType,0)<0){
                perror("Failed to receive message queue");
                exit(EXIT_FAILURE);
            }
            totCount=msRcv.mValue;
        }
        msgctl(msid,IPC_RMID,NULL);
    }
    
    gettimeofday(&timeEnd,NULL);
    timeDifVal=THOUSAND * (timeEnd.tv_sec-timeStart.tv_sec) + (timeEnd.tv_usec - timeStart.tv_usec)/THOUSAND;
    difT=timeDifVal;
    
    totalNumberOfMatchUp(totCount,argv[1],timeDifVal,exCon);
    
    return (EXIT_SUCCESS);
}
void isDirectoryOrFile(char *srcStr, char *dirName,int mid){
    DIR *dirPtr; /* directory acmak veya kapatmak icin pointer */
    struct dirent *dir; /* acilan directory icindekileri icinde barindirmaya 
                         yarayan struct dirent pointeri*/
    pid_t pid;
    pthread_t tid[THRD_SIZE];
    char temp[PATH_MAX],threadPath[PATH_MAX];
    int error,t=0,i=0;
    int threadSum=0;
    int total=0;
    int fileCount=0;
    FILE *searchDirPtr;
    FILE *searchFilePtr;
    FILE *totalThreadsPtr;
    FILE *maxThreadNumPtr;
    FILE *shmSizesPtr;
    struct msgBuf msg;

    dirPtr = opendir(dirName);
    strcpy(temp,dirName);/*directory path i gecici olarak bir array a kopyalama*/
    strcpy(threadPath,temp);
    sem_init(&sem,0,1);
    
    if(dirPtr){   
        while (((dir = readdir(dirPtr)) != NULL) && !doneflag){
            int length = strlen(dir->d_name);
            if (strncmp(dir->d_name + length - 1, "~", 1) != 0 
                    && dir->d_type==FILEVALUE) 
                fileCount++;
        }
        closedir(dirPtr);
    }
    
    shmSizesPtr=fopen(SHARED_MEM_SIZES,"a");
    if(fileCount==0){
        fprintf(shmSizesPtr,"%d\n",(int)sizeof(char));
        if ((id = shmget(IPC_PRIVATE,sizeof(char), PERM)) == -1) {
            perror("Failed to create shared memory segment");
            exit(EXIT_FAILURE);
        }  
    }else{
        fileCount *=sizeof(int);
        fprintf(shmSizesPtr,"%d\n",fileCount);
        if ((id = shmget(IPC_PRIVATE,fileCount, PERM)) == -1) {
            perror("Failed to create shared memory segment");
            exit(EXIT_FAILURE);
        } 
    }
    fclose(shmSizesPtr);
    if ((sharedTotal = (int *)shmat(id, NULL, 0)) == (void *)-1) {
        perror("Failed to attach shared memory segment");
        if (shmctl(id, IPC_RMID, NULL) == -1)
            perror("Failed to remove memory segment");
        exit(EXIT_FAILURE);
    }
    
    dirPtr = opendir(dirName);
    if (dirPtr)
    {   
        while (((dir = readdir(dirPtr)) != NULL) && !doneflag)
        {
            
            int length = strlen(dir->d_name);
            if (strncmp(dir->d_name + length - 1, "~", 1) != 0 
                    && dir->d_type==FILEVALUE) {
                
                strcat(threadPath, "/");
                strcat(threadPath,dir->d_name);
                
                searchFilePtr=fopen(SEARCH_FILES,"a");
                totalThreadsPtr=fopen(TOTAL_THREADS,"a");
                fprintf(searchFilePtr,"1\n");
                fprintf(totalThreadsPtr,"1\n");
                fclose(searchFilePtr);
                fclose(totalThreadsPtr);
                
                
                
                sem_wait(&sem); /* semaphore giris kismi*/
                /*-------------- struct atamasi-------------------------*/
                strcpy(fonkArgs[t].str,srcStr);
                strcpy(fonkArgs[t].path,threadPath);
                strcpy(fonkArgs[t].fileName,dir->d_name);
                /*------------------------------------------------------*/
                strcpy(threadPath,temp);
                threadSum += 1;
                thData.x=t;
                thData.num=&total;
                if (error = pthread_create(&tid[t], NULL, searchStrWithThread, &thData)){
                    fprintf(stderr, "Failed to create thread: %s\n", strerror(error));
                    exit(EXIT_FAILURE);
                }
                
                t++;
            }             
            else{
                if(!(strcmp(dir->d_name,".")==0 || strcmp(dir->d_name, "..")==0) 
                         && dir->d_type==DIRECTORYVALUE){
                    
                    searchDirPtr=fopen(SEARCH_DIRECTORIES,"a");
                    fprintf(searchDirPtr,"1\n");
                    fclose(searchDirPtr);
                    
                    for(i=0; i<t;i++){
                        if(error = pthread_join(tid[i], NULL)==-1){
                            fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
                        }
                    }
                    if ((msqid = msgget(IPC_PRIVATE, PERM | IPC_CREAT | O_NONBLOCK)) == -1)
                        perror("Failed to create new private message queue");
                    
                    pid=fork();
                    if(pid==0){ /* child ise */
                        
                        isMain=0;
                        strcat(temp, "/");
                        strcat(temp,dir->d_name);

                        fprintf(stdout,"%s/",dir->d_name);

                        closedir(dirPtr);
                        isDirectoryOrFile(srcStr,temp,msqid); 
                        exit(EXIT_SUCCESS);
                    }
                    if(pid < 0){   /* fork() == -1 degeri dondururse */
                   	perror("Unsuccesful fork() function");
                        exit(EXIT_FAILURE);
                    }
                    if(pid > 0){
                        if(msgrcv(msqid,&msg,sizeof(struct msgBuf),msg.mType,0)<0){
                            perror("Failed to receive message queue");
                            exit(EXIT_FAILURE);
                        }
                        (*sharedTotal) += msg.mValue;
                    }
                    msgctl(msqid,IPC_RMID,NULL);

                }  
            }
        }
        
        if(!doneflag){
            fprintf(stdout,"  : %d threads\n",threadSum);
            maxThreadNumPtr=fopen(MAX_THREAD_NUM,"a");
            fprintf(maxThreadNumPtr,"%d\n",threadSum);
            fclose(maxThreadNumPtr);
        }
        
        while(wait(NULL)>0);
        
        for(i=0; i<t;i++){
            if(error = pthread_join(tid[i], NULL)==-1){
                fprintf(stderr, "Failed to join thread: %s\n", strerror(error));
                exit(EXIT_FAILURE);
            }
        }
        (*sharedTotal)+=total;
        msg.mValue=(*sharedTotal);
        msg.mType=1;
        if(msgsnd(mid,&msg,sizeof(struct msgBuf),IPC_NOWAIT)<0){
            perror("Failed to send message queue");
            exit(EXIT_FAILURE);
        }
        if (detachandremove(id, sharedTotal) == -1) {
            perror("Failed to destroy shared memory segment");
            exit(EXIT_FAILURE);
        }
        sem_destroy(&sem);
        closedir(dirPtr); 
    }
    else{
        
        fprintf(stderr," SUCH A DIRECTORY COULD NOT BE FOUND !!! \n");
        while(wait(NULL)>0);
        totalNumberOfMatchUp(totCount,searchStr,difT,strerror(!dirPtr));
        
        shmdt(sharedTotal);
        shmctl(id, IPC_RMID, NULL);
        msgctl(msqid,IPC_RMID,NULL);
        msgctl(msid,IPC_RMID,NULL);
		remove(THREAD_IDS);
		remove(SEARCH_DIRECTORIES);
		remove(SEARCH_FILES);
		remove(TOTAL_THREADS);
		remove(SEARCH_LINES);
		remove(MAX_THREAD_NUM);
		remove(SHARED_MEM_SIZES);
	
        exit(EXIT_FAILURE);
    }
    
}
void readFileAndFindStr(char *srcStr,char *filePath, char *file,int* total)
{
    FILE *ptrFile; /* file acmak veya kapatmak icin pointer*/
    FILE *searchLinesPtr;
    char ch,chLine,temp; 
    int col=1,row=1,count=0,flag=1,spcCnt=0,chCount=0;
    int i=0,lineCount=0;
    ptrFile = fopen(filePath,"r");
    while((chLine = getc(ptrFile))!=EOF){
            if(chLine=='\n')
                lineCount++;
    }
    fclose(ptrFile);
    
    if(!doneflag){
        searchLinesPtr = fopen(SEARCH_LINES,"a");
        fprintf(searchLinesPtr,"%d\n",lineCount);
        fclose(searchLinesPtr);
    }
    
    ptrFile = fopen(filePath, "r"); 
    if(ptrFile==NULL)
    {
        
        fprintf(stderr, " UNABLE TO OPEN FILE!!! \n" );
        while(wait(NULL)>0);
        totalNumberOfMatchUp(totCount,searchStr,difT,strerror(!ptrFile));
        remove(THREAD_IDS);
		remove(SEARCH_DIRECTORIES);
		remove(SEARCH_FILES);
		remove(TOTAL_THREADS);
		remove(SEARCH_LINES);
		remove(MAX_THREAD_NUM);
		remove(SHARED_MEM_SIZES);
        exit(EXIT_FAILURE);
    }
    else
    {
        /*dosya sonuna gelene kadar tüm karakterleri oku:*/
        while ((ch = getc(ptrFile))!=EOF && flag==1){
            chCount++;
            if(ch=='\n'){
                row++;
                col=0;
            }
            i=0;
            /*aranan kelimenin ilk harfi ile dosyadan okunan esitse donguye 
             girer ve aranan kelime kadar okumak ister eger aranan kelimenin 
             kararkter sayisi kadar okunan karakter esitse i degeri aranan 
             kelimenin karakter sayisi kadar olur*/
            while(ch==srcStr[i]){   
                ch=getc(ptrFile);
                while(ch=='\n' || ch==' ' || ch=='\t'){
                    temp=ch;
                    ch=getc(ptrFile);
                    if((temp==' '|| temp=='\t')&& ch=='\n')
                        ch=getc(ptrFile);
                    if(ch==EOF)
                        flag=0;
                    spcCnt++; 
                }
                i++;
            }
            /* karakterler esitse i nin degeri kelime sayisina esitmi diye 
             * kontrol edilir*/
            if(i==strlen(srcStr)){
                count++; /* esit oldugunda count 1 artar*/
                printLog((long)getpid(),(long)pthread_self(),file,row,col,srcStr); /* log dosyasina basilir */
                /*fprintf(stdout,"%s: [%d, %d] %s first character is found.\n",filePath,
            row,col,srcStr);*/
            }
            col++;
            chCount += (i+spcCnt);
            fseek(ptrFile,-(chCount-1),SEEK_CUR);
            chCount=0;
            spcCnt=0; 
        }
    }
    
    *total += count;
    
    

    fclose(ptrFile); /* dosya kapatilir */
}
void printLog(long pid,long tid,char *fileName,int row,int col,char *srcString){
    FILE *logPtr; /* log.log dosyasi olusturmak icin pointer */
    logPtr = fopen(LOGFILE,"a"); /* log dosyasini append modu ile acilir */
    fprintf(logPtr,"ProcessID: %ld - ThreadID: %ld filename: %s: [%d, %d] %s first character is found.\n",pid,tid,fileName,
            row,col,srcString);
    fflush(logPtr);
    fclose(logPtr);        
}
void totalNumberOfMatchUp(int count,char *srcStr,double timedif,char*exitCondition){
    char ch;
    int line=0;
    int maxTh=0,th=0;
    int sharedNum=0,sharedSizes=0;
    int flag=1;
    FILE *logPtr;
    FILE *searchDirPtr;
    FILE *searchFilePtr;
    FILE *totalThreadsPtr;
    FILE *searchLinesPtr;
    FILE *maxThreadNumPtr;
    FILE *shmSizePtr;
    /*--------------------------------------------------------------------------------------*/
    if(count==0){
        logPtr = fopen(LOGFILE,"r");
        if(logPtr==NULL)
            logPtr = fopen(LOGFILE,"a");
	while((ch = getc(logPtr))!=EOF){
            
            if(ch=='\n')
                count++;
	}
	fclose(logPtr);
    }    
    logPtr = fopen(LOGFILE,"a");
    
    fprintf(logPtr,"------------------------------------------------------------\n");
    fprintf(stdout,"\n------------------------------------------------------------\n");
    fprintf(logPtr,"Total number of strings found: %d  (search string: %s)\n",count,srcStr);
    fprintf(stdout,"Total number of strings found: %d  (search string: %s)\n",count,srcStr);
    fclose(logPtr);
    /*--------------------------------------------------------------------------------------*/
    

    count=0;
    
        searchDirPtr = fopen(SEARCH_DIRECTORIES,"r");
        if(searchDirPtr==NULL)
            searchDirPtr = fopen(SEARCH_DIRECTORIES,"a");
	while((ch = getc(searchDirPtr))!=EOF){
            if(ch=='\n')
                count++;
	}
	fclose(searchDirPtr);
    logPtr = fopen(LOGFILE,"a");
    fprintf(logPtr,"Number of directories searched: %d\n",count+1);
    fprintf(stdout,"Number of directories searched: %d\n",count+1);
    fclose(logPtr);
    sharedNum=count+1;
    /*--------------------------------------------------------------------------------------*/
    count=0;
        searchFilePtr = fopen(SEARCH_FILES,"r");
        if(searchFilePtr==NULL)
            searchFilePtr = fopen(SEARCH_FILES,"a");
	while((ch = getc(searchFilePtr))!=EOF){
            if(ch=='\n')
                count++;
	}
	fclose(searchFilePtr);
    logPtr = fopen(LOGFILE,"a");
    fprintf(logPtr,"Number of files searched: %d\n",count);
    fprintf(stdout,"Number of files searched: %d\n",count);
    fclose(logPtr);
    /*--------------------------------------------------------------------------------------*/
    count=0;
        searchLinesPtr = fopen(SEARCH_LINES,"r");
        if(searchLinesPtr==NULL)
            searchLinesPtr = fopen(SEARCH_LINES,"a");
	while((fscanf(searchLinesPtr,"%d",&line))!=EOF)
            count += line;
	fclose(searchLinesPtr);
        
    logPtr = fopen(LOGFILE,"a");
    fprintf(logPtr,"Number of lines searched: %d\n",count);
    fprintf(stdout,"Number of lines searched: %d\n",count);
    fclose(logPtr);
    /*--------------------------------------------------------------------------------------*/
    count=0;
        totalThreadsPtr = fopen(SEARCH_FILES,"r");
        if(totalThreadsPtr==NULL)
            totalThreadsPtr = fopen(SEARCH_FILES,"a");
	while((ch = getc(totalThreadsPtr))!=EOF){
            if(ch=='\n')
                count++;
	}
	fclose(totalThreadsPtr);
    logPtr = fopen(LOGFILE,"a");
    fprintf(logPtr,"Number of search threads created: %d\n",count);
    fprintf(stdout,"Number of search threads created: %d\n",count);
    fclose(logPtr);
    /*--------------------------------------------------------------------------------------*/
    
        maxThreadNumPtr = fopen(MAX_THREAD_NUM,"r");
        if(maxThreadNumPtr==NULL)
            maxThreadNumPtr = fopen(MAX_THREAD_NUM,"a");
	while((fscanf(maxThreadNumPtr,"%d",&th))!=EOF)
            if(th>maxTh)
                maxTh=th;
	fclose(maxThreadNumPtr);
        
    logPtr = fopen(LOGFILE,"a");
    fprintf(logPtr,"Max # of threads running concurrently: %d\n",maxTh);
    fprintf(stdout,"Max # of threads running concurrently: %d\n",maxTh);
    fclose(logPtr);
    /*--------------------------------------------------------------------------------------*/
     /*--------------------------------------------------------------------------------------*/
    
        shmSizePtr = fopen(SHARED_MEM_SIZES,"r");
        if(shmSizePtr==NULL)
            shmSizePtr = fopen(SHARED_MEM_SIZES,"a");
        logPtr = fopen(LOGFILE,"a");
        fprintf(logPtr,"Total Shared Memory Number: %d & Shared Memorys Sizes: ",sharedNum);
        fprintf(stdout,"Total Shared Memory Number: %d & Shared Memorys Sizes: ",sharedNum);
        
	while((fscanf(shmSizePtr,"%d",&sharedSizes))!=EOF){
            
            fprintf(logPtr," %d,",sharedSizes);
            fprintf(stdout," %d,",sharedSizes);
            

        }
        fprintf(logPtr,"\n");
        fprintf(stdout,"\n");
        fclose(logPtr);
	fclose(shmSizePtr);
        
        
    /*--------------------------------------------------------------------------------------*/
    logPtr = fopen(LOGFILE,"a");
    fprintf(logPtr,"Total run time, %.4f in milliseconds. \n",fabs(timedif));
    fprintf(stdout,"Total run time, %.4f in milliseconds. \n",fabs(timedif));
    fprintf(logPtr,"Exit Condition: %s\n",exitCondition);
    fprintf(stdout,"Exit Condition: %s\n",exitCondition);
    fprintf(logPtr,"------------------------------------------------------------\n");
    fprintf(stdout,"\n------------------------------------------------------------\n");
    fclose(logPtr);
    
    
    remove(THREAD_IDS);
    remove(SEARCH_DIRECTORIES);
    remove(SEARCH_FILES);
    remove(TOTAL_THREADS);
    remove(SEARCH_LINES);
    remove(MAX_THREAD_NUM);
    remove(SHARED_MEM_SIZES);
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

