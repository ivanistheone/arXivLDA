/*---------------------------------------------------
 * file:    topiclib.h
 * purpose: header file for topiclib.c
 * usage:   include in *.c
 * version: 1.0
 * author:  David Newman, newman@uci.edu
 * date:    11/19/08
 *-------------------------------------------------*/

#ifndef _TOPICLIB_H_
#define _TOPICLIB_H_
 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

int **imat(int nr, int nc);
double **dmat(int nr, int nc);
int *ivec(int n);
double *dvec(int n);
int imax(int n, int *x);
double dmax(int n, double *x);
int imin(int n, int *x);
double dmin(int n, double *x);
int isum(int n, int *x);
double dsum(int n, double *x);
double ddot(int n, double *x, double *y);
int countlines(char *fname);
int countN(char *fname);
void isort(int n, int *x, int direction, int *indx);
int countnnz(int nr, int nc, int **x);
void write_sparse(int nr, int nc, int **x, char *fname);
int **read_sparse(char *fname, int *nr_, int *nc_);
int **read_sparse_trans(char *fname, int *nc_, int *nr_);
void read_dw(char *fname, int *d, int *w, int *D, int *W);
void fill_dtot(int N, int *d, int *dtot);
void read_dwc(char *fname, int *d, int *w, int *c, int *D, int *W);
int read_nnz(char *fname);
void write_ivec (int n, int *x, char *fname);
void read_ivec (int n, int *x, char *fname);
int *randperm(int n);
void randomassignment(int N, int T, int *w, int *d, int *z, int **wp, int **dp, int *ztot);
void sample_chain (int N, int W, int T, double alpha, double beta, int *w, int *d, int *z, int **wp, int **dp, int *ztot, int *order);
void resample_chain (int N, int W, int T, double alpha, double beta, int *w, int *d, int *z, int **wp, int **dp, int *ztot);
void loglike (int N, int W, int D, int T, double alpha, double beta, int *w, int *d, int **wp, int **dp, int *ztot, int *dtot);
void chksum (int n, int T, int **x, int *sumx);

#endif
