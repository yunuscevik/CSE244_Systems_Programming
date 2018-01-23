/* 
 * File:   main.c
 * Author: Yunus ÇEVİK
 *
 * Created on 04 Mart 2017 Cumartesi, 10:35
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
void readFile(char *srcStr, char *file);
int main(int argc, char** argv) {
    if(argc !=3)
    {
        fprintf(stderr, " Usage: ./list 'string'  <filename> \n" );
        exit(EXIT_FAILURE);
    }
    readFile(argv[1],argv[2]);
    return (EXIT_SUCCESS);
}
void readFile(char *srcStr, char *file)
{
    FILE *ptrFile;
    char ch,temp;
    int col=1,row=1,count=0,flag=1,spcCnt=0,chCount=0;
    ptrFile = fopen(file, "r");
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
            if(i==strlen(srcStr)){
                count++;
                printf("[%d, %d] konumunda ilk karakter bulundu\n",row,col);
            }
            col++;
            chCount += (i+spcCnt);
            fseek(ptrFile,-(chCount-1),SEEK_CUR);
            chCount=0;
            spcCnt=0; 
        }
    }
    if(count>0)
        printf("%d adet %s bulundu.\n",count,srcStr);
    else
        printf("%d adet %s bulundu.\n",count,srcStr);
    fclose(ptrFile);
}
