
/* 
 * File:   matrixMethods.h
 * Author: YunusCevik
 *
 * Created on 29 Mayıs 2017 Pazartesi, 18:12
 */

#ifndef MATRIXMETHODS_H
#define MATRIXMETHODS_H
#define SIZE 50
#define ZERO 1.0E-20
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
void gaussianElimination(double matrix[][SIZE],double bMatrix[SIZE],double xMatrix[SIZE],int n);
void pseudoInverse(double  a[][50],double b[][50],double x[][50],int m,int p);
double determinant(double matrix[][50],int n);
void transpose (double matrix[][50],double transposedMatrix[][50],int m, int p);
void inverseMatrix(double matrix[][50],double inversed[][50],int n);
void matrixMult(double resultMatrix[][50], double matrix1[][50], double matrix2[][50],int m,int p,int p2);

#endif /* MATRIXMETHODS_H */

