#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <wait.h>
#define SEEK_CUR 1
#define LOGFILE "log.log"
#define TEMPFILE "temp.log"
#define FILEVALUE 8
#define DIRECTORYVALUE 4
#define ARGCVALUE 3
void readFileAndFindStr(char *srcStr,char *filePath, char *file);
void isDirectoryOrFile(char *srcStr, char *dirName);
void printLog(char *fileName,int row,int col,char *srcString);
void writeTempFile(int a);
int readTempFile();
void totalNumberOfMatchUp(int count,char *srcStr);

int main(int argc, char** argv) {
    int count=0;
    if(argc !=ARGCVALUE)
    {
        fprintf(stderr, "Usage: ./listdir 'string'  <directoryName> \n");
        exit(EXIT_FAILURE);
    }
    remove(LOGFILE);
    char path[PATH_MAX];
    getcwd(path,PATH_MAX);
    strcat(path,"/");
    strcat(path,argv[2]);
    isDirectoryOrFile(argv[1],path);
    count=readTempFile();
    totalNumberOfMatchUp(count,argv[1]);
    return (EXIT_SUCCESS);
}
void isDirectoryOrFile(char *srcStr, char *dirName){
    DIR *dirPtr; /* directory acmak veya kapatmak icin pointer */
    struct dirent *dir; /* acilan directory icindekileri icinde barindirmaya 
                         yarayan struct dirent pointeri*/
    pid_t pid;  
    dirPtr = opendir(dirName);
    char temp[PATH_MAX];
    strcpy(temp,dirName);/*directory path i gecici olarak bir array a kopyalama*/
    if (dirPtr)
    {   
        while ((dir = readdir(dirPtr)) != NULL)
        {
            int length = strlen(dir->d_name);
            if (strncmp(dir->d_name + length - 1, "~", 1) != 0 
                    && dir->d_type==FILEVALUE) {
                pid=fork(); /*dosyaysa fork kullanilarak parent ve child olusturulur*/
                if (pid == 0){ /* child ise */
                    strcat(temp, "/");
                    strcat(temp,dir->d_name);
                    readFileAndFindStr(srcStr,temp,dir->d_name);
                    closedir(dirPtr);
                    exit(EXIT_SUCCESS);
                }
                else if(pid > 0) /* parent ise */
                    wait(NULL);
                else    /* fork() == -1 degeri dondururse */
                    perror("Unsuccesful fork() function");

            } 
            else{
                if(!(strcmp(dir->d_name,".")==0 || strcmp(dir->d_name, "..")==0) 
                         && dir->d_type==DIRECTORYVALUE){
                    pid=fork();
                    if(pid==0){ /* child ise */
                        strcat(temp, "/");
                        strcat(temp,dir->d_name);
                        closedir(dirPtr);
                        isDirectoryOrFile(srcStr,temp);
                        exit(EXIT_SUCCESS);
                    }
                    else if (pid > 0) /* parent ise */
                        wait(NULL);
                    else    /* fork() == -1 degeri dondururse */
                   	perror("Unsuccesful fork() function");
                }
            }
        }
        closedir(dirPtr);    
    }
	else{
        fprintf(stderr," SUCH A DIRECTORY COULD NOT BE FOUND !!! \n");
        exit(EXIT_FAILURE);
    }
}
void readFileAndFindStr(char *srcStr,char *filePath, char *file)
{
    FILE *ptrFile; /* file acmak veya kapatmak icin pointer*/
    char ch,temp; 
    int col=1,row=1,count=0,flag=1,spcCnt=0,chCount=0;
    ptrFile = fopen(filePath, "r"); /*dosyayi sadece okuma modunda acma*/
    if(ptrFile==NULL)
    {
        fprintf(stderr, " UNABLE TO OPEN FILE!!! \n" );
        exit(EXIT_FAILURE);
    }
    else
    {
        // dosya sonuna gelene kadar tüm karakterleri oku:
        while ((ch = getc(ptrFile))!=EOF && flag==1){
            chCount++;
            if(ch=='\n'){
                row++;
                col=0;
            }
            int i=0;
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
                //fprintf(stdout,"[%d, %d] konumunda ilk karakter bulundu\n",row,col);
            }
            col++;
            chCount += (i+spcCnt);
            fseek(ptrFile,-(chCount-1),SEEK_CUR);
            chCount=0;
            spcCnt=0; 
        }
    }
    /* dosyalarda bulunan kelime adetleri gecici log dosyasina yazilir */
    writeTempFile(count); 
    fclose(ptrFile); /* dosya kapatilir */
}
void printLog(char *fileName,int row,int col,char *srcString)
{
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
    remove(TEMPFILE);
}
void writeTempFile(int a){
    FILE *fPtr;
    fPtr= fopen(TEMPFILE,"a");
    fprintf(fPtr,"%d\n",a);
    fclose(fPtr);      
}
int readTempFile(){
    int totalCount=0,num=0;
    FILE *fPtr;
    fPtr= fopen(TEMPFILE,"r");
    while(fscanf(fPtr,"%d",&num)!=EOF)
        totalCount += num;
    fclose(fPtr);
    return totalCount;
}
