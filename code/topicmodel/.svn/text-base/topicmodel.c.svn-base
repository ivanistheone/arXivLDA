/*---------------------------------------------------
 * file:    topicmodel.c
 * purpose: run collapsed Gibbs sampled LDA (topic model)
 * usage:   topicmodel T iter
 * inputs:  T (number of topics), iter (number of iterations), docword.txt
 * outputs: wp.txt, dp.txt, z.txt
 * version: 1.0
 * author:  David Newman, newman@uci.edu
 * date:    11/19/08
 *-------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "topiclib.h"

/*==========================================
 * main
 *========================================== */
int main(int argc, char* argv[])
{
  int i=0, iter=0, seed=1;
  int *w, *d, *z, **wp, **dp, *ztot, *dtot, *order;
  double alpha;
  double beta = 0.01;

  int T; // number of topics
  int W; // number of unique words
  int D; // number of docs
  int N; // number of words in corpus
  
  if (argc == 1) {
    fprintf(stderr, "usage: %s T iter\n", argv[0]);
    exit(-1);
  }
  T    = atoi(argv[1]);
  iter = atoi(argv[2]);
  
  assert(T>0);
  assert(iter>0);
  assert(beta>0);
  assert(seed>0);

  N = countN("docword.txt");
  w = ivec(N);
  d = ivec(N);
  z = ivec(N);
  read_dw("docword.txt", d, w, &D, &W);
  alpha = 0.1*N/(D*T);

  printf("memory= %.3fGB\n", (4*N+T*(D+W))*sizeof(int)/1e9);
  printf("seed  = %d\n", seed);
  printf("N     = %d\n", N);
  printf("W     = %d\n", W);
  printf("D     = %d\n", D);
  printf("T     = %d\n", T);
  printf("iter  = %d\n", iter);
  printf("alpha = %f\n", alpha);
  printf("beta  = %f\n", beta);
  
  dtot = ivec(D);
  fill_dtot(N, d, dtot);
  assert(isum(D,dtot)==N);
  wp = imat(W,T);
  dp = imat(D,T);
  ztot = ivec(T);  
  
  srand48(seed);
  randomassignment(N,T,w,d,z,wp,dp,ztot);
  order = randperm(N);
  
  for (i = 0; i < iter; i++) {
    sample_chain(N,W,T,alpha,beta,w,d,z,wp,dp,ztot,order);
    printf("iter %d \n", i);
  }
  
  write_sparse(W,T,wp,"wp.txt");
  write_sparse(D,T,dp,"dp.txt");
  write_ivec  (N,  z,  "z.txt");

  //loglike(N,W,D,T,alpha,beta,w,d,wp,dp,ztot,dtot);
  
  return 0;
}
