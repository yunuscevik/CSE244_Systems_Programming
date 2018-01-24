
#include "matrixMethods.h"


void pseudoInverse(double  a[][50],double b[][50],double x[][50],int m,int p){
    double transposeA[50][50],inversedM[50][50],multiplicationMinat[50][50];
    double multiplicationMAtA[50][50];
    transpose(a,transposeA,m,p);
    matrixMult(multiplicationMAtA,transposeA,a,p,m,p);
    inverseMatrix(multiplicationMAtA,inversedM,p);
    matrixMult(multiplicationMinat,inversedM,transposeA,p,p,m);
    matrixMult(x,multiplicationMinat,b,p,m,1);
}

void gaussianElimination(double matrix[][SIZE],double bMatrix[SIZE],double xMatrix[SIZE],int n){
    int i,j,k,flag=1,iMax,temp=0;
    double *s,maxVal=0.0,aMax,sum=0.0,val;
    int *nRow;
    nRow=(int *)malloc((n-1)*sizeof(int));
    s=(double*)malloc((n-1)*sizeof(double));
    for(i=0;i<n-1;i++){
        maxVal=fabs(matrix[i][0]);
        for(j=0;j<n-1;j++){
            if(j==n-1)
                bMatrix[i]=matrix[i][j];
            else{
                if(maxVal < fabs(matrix[i][j]))
                    maxVal = fabs(matrix[i][j]); 
            }
        }
        s[i]=maxVal;
        if(s[i]==0){
            printf("No enique solution exists !!!\n");
            exit(EXIT_FAILURE);
        }
        nRow[i]=i+1;
    }
    
    i=1;
    while(flag==1 && i<=n-2){
        iMax=nRow[i-1];
        aMax=fabs(matrix[iMax-1][i-1]) / s[iMax-1];
        iMax=i;
        for(j=i+1;j<=n-1;j++){
            if(fabs(matrix[nRow[j-1]-1][i-1]) / s[nRow[j-1]-1] > aMax){
                aMax=fabs(matrix[nRow[j-1]-1][i-1]) / s[nRow[j-1]-1];
                iMax=j;  
            }
        }
        if(aMax<=ZERO){
            printf("No enique solution exists !!!\n");
            flag=0;
        }
        else{
            if(nRow[i-1] != nRow[iMax-1]){
                temp= nRow[i-1];
                nRow[i-1]=nRow[iMax-1];
                nRow[iMax-1]=temp; 
            }
            for(j= i+1;j<=n-1;j++){
                val = matrix[nRow[j-1]-1][i-1] / matrix[nRow[i-1]-1][i-1];
                for(k=i+1;k<=n;k++){   
                    matrix[nRow[j-1]-1][k-1] = matrix[nRow[j-1]-1][k-1] - val * matrix[nRow[i-1]-1][k-1];  
                }
                matrix[nRow[j-1]-1][i-1] = 0.0;
            }
        }
        i++;
    }
    if(flag == 1){
        if(fabs(matrix[nRow[n-2]-1][n-2])<=ZERO){
            printf("No enique solution exists !!!");
            flag=0;
        }
        else{
            xMatrix[n-2] = matrix[nRow[n-2]-1][n-1] / matrix[nRow[n-2]-1][n-2];
            for(k=1;k<=n-2;k++){
                i=n-2-k+1;
                sum=0.0;
                for(j=i+1;j<=n-1;j++)
                   sum = sum - matrix[nRow[i-1]-1][j-1] * xMatrix[j-1];
                xMatrix[i-1] = (matrix[nRow[i-1]-1][n-1] + sum) / matrix[nRow[i-1]-1][i-1];
                
            }
		}
            
    }
    free(s);
    free(nRow); 
}




























/*
 * http://easy-learn-c-language.blogspot.com.tr/2013/02/numerical-methods-determinant-of-nxn.html
 * dan yararlanılmıştır.
 */
double determinant(double matrix[][50],int n){
/* Conversion of matrix to upper triangular */
    int i,j,k;
    double det,ratio,temp[50][50];
    for(i = 0; i < n; i++)
        for(j = 0; j < n; j++)
            temp[i][j] = matrix[i][j];
        
    
    
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(j>i){
                ratio = temp[j][i]/temp[i][i];
                for(k = 0; k < n; k++){
                    temp[j][k] -= ratio * temp[i][k];
                }
            }
        }
    }
    det = 1;
    for(i = 0; i < n; i++)
        det *= temp[i][i];
    return det;
}

void transpose (double matrix[][50],double transposedMatrix[][50],int m, int p){
    int i,j;
    for(i=0 ; i < p ; i++){
        for(j=0; j < m ; j++)
            transposedMatrix[i][j]=matrix[j][i]; 
    }
    
}
/*
 * http://www.programming-techniques.com/2011/09/numerical-methods-inverse-of-nxn-matrix.html
 * Yukarıdaki linkteki kod uygun şekilde değiştirilerek kullanılmıştır.
 */
void inverseMatrix(double matrix[][50],double inversed[][50],int n){
    int i,j,k;
    double a,ratio,temp[50][50];
    for (i=0;i<n;++i)
        for(j=0;j<n;++j)
            temp[i][j] = matrix[i][j];
    
    for(i = 0; i < n; i++){
        for(j = n; j < 2*n; j++){
            if(i==(j-n))
                temp[i][j] = 1.0;
            else
                temp[i][j] = 0.0;
        }
    }
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(i!=j){
                ratio = temp[j][i]/temp[i][i];
                for(k = 0; k < 2*n; k++){
                    temp[j][k] -= ratio * temp[i][k];
                }
            }
        }
    }
    for(i = 0; i < n; i++){
        a = temp[i][i];
        for(j = 0; j < 2*n; j++){
            temp[i][j] /= a;
        }
    }
    k = n;
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            inversed[i][j] = temp[i][k];
            k++;
        }
        k = n;
    }
}
/*
 * https://www.programiz.com/c-programming/examples/matrix-multiplication
 * sitesinden yararlanılmıştır.
 */
void matrixMult(double resultMatrix[][50], double matrix1[][50], double matrix2[][50],int m,int p,int p2){
    int i,j,k;
    for(i=0; i<m; ++i)
        for(j=0; j<p2; ++j)
            for(k=0; k<p; ++k)
                resultMatrix[i][j]+=matrix1[i][k]*matrix2[k][j];
}
