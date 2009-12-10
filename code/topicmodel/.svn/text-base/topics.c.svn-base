/*---------------------------------------------------
 * file:    topics.c
 * purpose: compute topics
 * usage:   topics
 * inputs:  wp.txt, vocab.txt
 * outputs: topics.txt
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
  int i, t, w, **tw, *ztot;
  //double beta = 0.01;
  char **word;
  FILE *fp;
  int *prob_w_given_t, *indx;

  int T; // number of topics
  int W; // number of unique words
  
  if (argc != 1) {
    fprintf(stderr, "usage: %s\n", argv[0]);
    exit(-1);
  }
  
  tw = read_sparse_trans("wp.txt",&T,&W);  assert(T>0); assert(W>0);
  fp = fopen("vocab.txt","r");             assert(fp);
  word = (char**)malloc(W*sizeof(char*));  assert(word);
  for (w = 0; w < W; w++) {
    word[w] = (char*)malloc(100*sizeof(char));
    fscanf(fp,"%s",word[w]);
  }
  fclose(fp);

  prob_w_given_t = ivec(W);
  indx           = ivec(W);
  ztot           = ivec(T);

  for (t = 0; t < T; t++) {
    ztot[t] = 0;
    for (w = 0; w < W; w++) ztot[t] += tw[t][w];
  }
  
  for (t = 0; t < T; t++) {
    
    for (w = 0; w < W; w++) prob_w_given_t[w] = tw[t][w];
    isort(W, prob_w_given_t, -1, indx);
   
    printf("[t%d] ", t+1);
    for (i = 0; i < 12; i++) printf("%s ", word[indx[i]]);
    printf("...\n");
    
  }
  
  return 0;
}
