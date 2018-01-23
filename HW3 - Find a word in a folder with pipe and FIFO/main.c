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

#define SEEK_CUR 1
#define LOGFILE "log.log"
#define FILEVALUE 8
#define DIRECTORYVALUE 4
#define ARGCVALUE 3
#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define PATH_MAX 4096
#define PIPE_BUF 4096
#define FIFONAME "myfifo"

void readFileAndFindStr(char *srcStr,char *filePath, char *file,int *fd);
void isDirectoryOrFile(char *srcStr, char *dirName,int fifoFd);
void printLog(char *fileName,int row,int col,char *srcString);
void totalNumberOfMatchUp(int count,char *srcStr);


int fifoNum=0;
int main(int argc, char** argv) {
    pid_t pid;
    int count=0,fd[2];
    char path[PATH_MAX],mainFifo[PIPE_BUF];
    
    if(argc !=ARGCVALUE)
    {
        fprintf(stderr, "Usage: ./withPipeandFIFO 'string'  <directoryName> \n");
        exit(EXIT_FAILURE);
    }
    remove(LOGFILE);
    getcwd(path,PATH_MAX);
    strcat(path,"/");
    strcat(path,argv[2]);
    strcpy(mainFifo,path);
    strcat(mainFifo,"/");
    strcat(mainFifo,FIFONAME);
    
    if (mkfifo(mainFifo, FIFO_PERMS) == -1) { 
        if (errno != EEXIST) {
            fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
            (long)getpid(), mainFifo, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    pid=fork();
    if(pid==0){
        while (((fd[1] = open(mainFifo, O_WRONLY )) == -1) && (errno == EINTR)) ;
        if (fd[1] == -1) {
            fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
            (long)getpid(), mainFifo, strerror(errno));
            exit(EXIT_FAILURE);
        }
        isDirectoryOrFile(argv[1],path,fd[1]);
        close(fd[1]);
        exit(EXIT_SUCCESS);
    }
    else if(pid>0){
        
        fd[0] = open(mainFifo, O_RDONLY) ;   
        if (fd[0] == -1) {
            fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
            (long)getpid(), mainFifo, strerror(errno));
            exit(EXIT_FAILURE);
        }
        while(read(fd[0],&count,sizeof(int))>0);
        close(fd[0]);
    }
    unlink(mainFifo);
    totalNumberOfMatchUp(count,argv[1]);
    return (EXIT_SUCCESS);
}
void isDirectoryOrFile(char *srcStr, char *dirName ,int fifoFd ){
    DIR *dirPtr; /* directory acmak veya kapatmak icin pointer */
    struct dirent *dir; /* acilan directory icindekileri icinde barindirmaya 
                         yarayan struct dirent pointeri*/
    pid_t pid;
    int cntr=0;
    int count=0,result=0;
    int fileDesP[2],fileDesF[2];
    char temp[PATH_MAX],fifo[PIPE_BUF];
    dirPtr = opendir(dirName);
    strcpy(temp,dirName);/*directory path i gecici olarak bir array a kopyalama*/
    strcpy(fifo,temp);
    strcat(fifo,"/");
    strcat(fifo,FIFONAME);
    
    if (dirPtr)
    {   
        while ((dir = readdir(dirPtr)) != NULL)
        {
            int length = strlen(dir->d_name);
            if (strncmp(dir->d_name + length - 1, "~", 1) != 0 
                    && dir->d_type==FILEVALUE) {
                
                if(pipe(fileDesP)<0){
                    perror("Unsuccesful pipe() function");
                    exit(EXIT_FAILURE);
                }
                pid=fork(); /*dosyaysa fork kullanilarak parent ve child olusturulur*/
                if (pid == 0){ /* child ise */
                    close(fileDesP[0]);
                    strcat(temp, "/");
                    strcat(temp,dir->d_name);
                    readFileAndFindStr(srcStr,temp,dir->d_name,fileDesP);
                    close(fileDesP[1]);
                    closedir(dirPtr);
                    exit(EXIT_SUCCESS);
                }
                if (pid<0){   /* fork() == -1 degeri dondururse */
                    perror("Unsuccesful fork() function");
                    exit(EXIT_FAILURE);
                }
                if(pid>0){ /* parent ise */
                    close(fileDesP[1]);
                    while(read(fileDesP[0],&count,sizeof(int))>0);
                        cntr += count;
                    close(fileDesP[0]);

                }
            }             
            else{
                if(!(strcmp(dir->d_name,".")==0 || strcmp(dir->d_name, "..")==0) 
                         && dir->d_type==DIRECTORYVALUE){
                    
                    strcat(fifo,dir->d_name);
                    if (mkfifo(fifo, FIFO_PERMS) == -1) { 
                        if (errno != EEXIST) {
                            fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
                            (long)getpid(), fifo, strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                    }
                    pid=fork();
                    if(pid==0){ /* child ise */
                        while (((fileDesF[1] = open(fifo, O_WRONLY )) == -1) && (errno == EINTR)) ;
                        if (fileDesF[1] == -1) {
                            fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
                            (long)getpid(), fifo, strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        strcat(temp, "/");
                        strcat(temp,dir->d_name);
                        closedir(dirPtr);
                        isDirectoryOrFile(srcStr,temp,fileDesF[1]);
                        close(fileDesF[1]);
                        exit(EXIT_SUCCESS);
                    }
                    if(pid < 0){   /* fork() == -1 degeri dondururse */
                   	perror("Unsuccesful fork() function");
                        exit(EXIT_FAILURE);
                    }
                    if(pid>0){ /* parent ise */
                        fileDesF[0] = open(fifo, O_RDONLY) ;
                        if (fileDesF[0] == -1) {
                            fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
                            (long)getpid(), fifo, strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        while(read(fileDesF[0],&result,sizeof(int))>0){
                            cntr += result;  
                        }
                        close(fileDesF[0]);
                        unlink(fifo);
                    }
                }  
            }          
        }
        if(write(fifoFd,&cntr,sizeof(int))>0);
        while(wait(NULL)>0);
        closedir(dirPtr); 
    }
    else{
        fprintf(stderr," SUCH A DIRECTORY COULD NOT BE FOUND !!! \n");
        exit(EXIT_FAILURE);
    }
}
void readFileAndFindStr(char *srcStr,char *filePath, char *file,int *fd)
{
    FILE *ptrFile; /* file acmak veya kapatmak icin pointer*/
    char ch,temp; 
    int col=1,row=1,count=0,flag=1,spcCnt=0,chCount=0;
    int i=0;
    ptrFile = fopen(filePath, "r"); /*dosyayi sadece okuma modunda acma*/
    if(ptrFile==NULL)
    {
        fprintf(stderr, " UNABLE TO OPEN FILE!!! \n" );
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
                printLog(file,row,col,srcStr); /* log dosyasina basilir */
            }
            col++;
            chCount += (i+spcCnt);
            fseek(ptrFile,-(chCount-1),SEEK_CUR);
            chCount=0;
            spcCnt=0; 
        }
    }
    /* dosyalarda bulunan kelime adetleri gecici log dosyasina yazilir */
    write(fd[1],&count,sizeof(int));
    
    fclose(ptrFile); /* dosya kapatilir */
}
void printLog(char *fileName,int row,int col,char *srcString){
    FILE *logPtr; /* log.log dosyasi olusturmak icin pointer */
    logPtr = fopen(LOGFILE,"a"); /* log dosyasini append modu ile acilir */
    fprintf(logPtr,"%s: [%d, %d] %s first character is found.\n",fileName,
            row,col,srcString);
    fclose(logPtr);        
}
void totalNumberOfMatchUp(int count,char *srcStr){
    FILE *logPtr;
    logPtr = fopen(LOGFILE,"a");
    fprintf(logPtr,"%d %s were found in total.\n",count,srcStr);
    fclose(logPtr);  
}
