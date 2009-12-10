/*---------------------------------------------------
 * file:    topiclib.c
 * purpose: utility routines
 * version: 1.0
 * author:  David Newman, newman@uci.edu
 * date:    11/19/08
 *-------------------------------------------------*/

// grep //$ topiclib.c

#include "topiclib.h"

/*------------------------------------------
 * private
 *------------------------------------------ */

static int icomp(const void *, const void *); /* comparison for isort */
static int    *icomp_vec;                     /*  data used for isort */

/*------------------------------------------
 * allocation routines
 * imat
 * dmat
 * ivec
 * dvec
 *------------------------------------------ */

int **imat(int nr, int nc) //
{
  int ntot = nr*nc;
  int *tmp = (int*) calloc(ntot,sizeof(int));
  int **x  = (int**)calloc(nr,sizeof(int*));
  int r;
  assert(tmp);
  assert(x);
  for (r = 0; r < nr; r++) x[r] = tmp + nc*r;
  return x;
}
  
double **dmat(int nr, int nc) //
{
  int ntot = nr*nc;
  double *tmp = (double*) calloc(ntot,sizeof(double));
  double **x  = (double**)calloc(nr,sizeof(double*));
  int r;
  assert(tmp);
  assert(x);
  for (r = 0; r < nr; r++) x[r] = tmp + nc*r;
  return x;
}
  
int *ivec(int n) //
{
  int *x = (int*)calloc(n,sizeof(int));
  assert(x);
  return x;
}
  
double *dvec(int n) //
{
  double *x = (double*)calloc(n,sizeof(double));
  assert(x);
  return x;
}

/*------------------------------------------
 * vector routines
 * imax
 * dmax
 *------------------------------------------ */

int imax(int n, int *x) //
{
  int i, xmax=x[0];
  for (i = 0; i < n; i++) xmax = MAX(xmax,x[i]);
  return xmax;
}

double dmax(int n, double *x) //
{
  int i;
  double  xmax=x[0];
  for (i = 0; i < n; i++) xmax = MAX(xmax,x[i]);
  return xmax;
}

int imin(int n, int *x) //
{
  int i, xmin=x[0];
  for (i = 0; i < n; i++) xmin = MIN(xmin,x[i]);
  return xmin;
}

double dmin(int n, double *x) //
{
  int i;
  double  xmin=x[0];
  for (i = 0; i < n; i++) xmin = MIN(xmin,x[i]);
  return xmin;
}

int isum(int n, int *x) //
{
  int i, xsum=0;
  for (i = 0; i < n; i++) xsum += x[i];
  return xsum;
}

double dsum(int n, double *x) //
{
  int i;
  double xsum=0;
  for (i = 0; i < n; i++) xsum += x[i];
  return xsum;
}

double ddot(int n, double *x, double *y) //
{
  int i;
  double xdot=0;
  for (i = 0; i < n; i++) xdot += x[i]*y[i];
  return xdot;
}

/*------------------------------------------
 * countlines
 * 
 *------------------------------------------ */
int countlines(char *fname) //
{
  int lines = 0;
  char buf[BUFSIZ];
  FILE *fp = fopen(fname ,"r"); assert(fp);
  while (fgets(buf, BUFSIZ, fp)) lines++;
  fclose(fp);
  lines -= 3; // less 3 header lines
  assert(lines>0);
  return lines;
}

int countN(char *fname) //
{
  int i, count, N = 0;
  char buf[BUFSIZ];
  FILE *fp = fopen(fname ,"r"); assert(fp);
  for (i = 0; i < 3; i++) fgets(buf, BUFSIZ, fp); // skip 3 header lines
  while (fscanf(fp, "%*d%*d%d", &count) != EOF) N += count;
  fclose(fp);
  assert(N>0);
  return N;
}

/*------------------------------------------
 * sort: call qsort library function
 * isort
 * dsort
 *------------------------------------------ */

void isort(int n, int *x, int direction, int *indx) //
{
  int i;
  assert(direction*direction==1);
  icomp_vec = ivec(n);
  for (i = 0; i < n; i++) {
    icomp_vec[i] = direction*x[i];
    indx[i] = i;
  }
  qsort(indx,n,sizeof(int),icomp);
  free(icomp_vec);
}
static int icomp(const void *pl, const void *p2)
{
  int i = * (int *) pl;
  int j = * (int *) p2;
  return (icomp_vec[i] - icomp_vec[j]);
}

/*------------------------------------------
 * io
 *------------------------------------------ */
int countnnz(int nr, int nc, int **x) //
{
  int i, j, nnz=0;
  for (i = 0; i < nr; i++) 
    for (j = 0; j < nc; j++) 
      if (x[i][j] > 0) nnz++;
  return nnz;
}

void write_sparse(int nr, int nc, int **x, char *fname) //
{
  FILE *fp = fopen(fname,"w");
  int i, j;
  assert(fp);
  fprintf(fp, "%d\n", nr);
  fprintf(fp, "%d\n", nc);
  fprintf(fp, "%d\n", countnnz(nr,nc,x));
  for (i = 0; i < nr; i++) 
    for (j = 0; j < nc; j++) 
      if (x[i][j] > 0) fprintf(fp, "%d %d %d\n", i+1 , j+1 , x[i][j]);
  fclose(fp);
}

int **read_sparse(char *fname, int *nr_, int *nc_) //
{
  FILE *fp = fopen(fname,"r");
  int i, j, c, nr, nc, nnz;
  int **x;
  assert(fp);
  fscanf(fp,"%d", &nr);  assert(nr>0);
  fscanf(fp,"%d", &nc);  assert(nc>0);
  fscanf(fp,"%d", &nnz); assert(nnz>0);
  x = imat(nr,nc);
  while (fscanf(fp, "%d%d%d", &i, &j, &c) != EOF) {
    i--;
    j--;
    assert(i<nr);
    assert(j<nc);
    assert(c>0);
    x[i][j] = c;
  }
  fclose(fp);
  *nr_ = nr;
  *nc_ = nc;
  return x;
}

int **read_sparse_trans(char *fname, int *nc_, int *nr_) //
{
  FILE *fp = fopen(fname,"r");
  int i, j, c, nr, nc, nnz;
  int **x;
  assert(fp);
  fscanf(fp,"%d", &nr);  assert(nr>0);
  fscanf(fp,"%d", &nc);  assert(nc>0);
  fscanf(fp,"%d", &nnz); assert(nnz>0);
  x = imat(nc,nr);
  while (fscanf(fp, "%d%d%d", &i, &j, &c) != EOF) {
    i--;
    j--;
    assert(i<nr);
    assert(j<nc);
    assert(c>0);
    x[j][i] = c;
  }
  fclose(fp);
  *nr_ = nr;
  *nc_ = nc;
  return x;
}

void read_dw(char *fname, int *d, int *w, int *D, int *W) //
{
  int i,wt,dt,ct,count,nnz;
  FILE *fp = fopen(fname ,"r"); assert(fp);
  count = 0;
  fscanf(fp,"%d", D);    assert(*D>0);
  fscanf(fp,"%d", W);    assert(*W>0);
  fscanf(fp,"%d", &nnz); assert(nnz>0);
  while (fscanf(fp, "%d%d%d", &dt, &wt, &ct) != EOF) {
    for (i = count; i < count+ct; i++) {
      w[i] = wt-1;
      d[i] = dt-1;
    }
    count += ct;
  }
  fclose(fp);
}

void fill_dtot(int N, int *d, int *dtot) //
{
  int i;
  for (i = 0; i < N; i++) dtot[d[i]]++;
}

void read_dwc(char *fname, int *d, int *w, int *c, int *D, int *W) //
{
  FILE *fp = fopen(fname,"r");
  int i=0, dd, ww, cc, nnz;
  assert(fp);
  fscanf(fp,"%d", D);    assert(*D>0);
  fscanf(fp,"%d", W);    assert(*W>0);
  fscanf(fp,"%d", &nnz); assert(nnz>0);
  while (fscanf(fp, "%d%d%d", &dd, &ww, &cc) != EOF) {
    d[i] = --dd;
    w[i] = --ww;
    c[i] = cc;
    i++;
  }
  assert(i==nnz);
  fclose(fp);
}

int read_nnz(char *fname) //
{
  int nnz;
  FILE *fp = fopen(fname,"r"); assert(fp);
  fscanf(fp,"%d", &nnz); // nr
  fscanf(fp,"%d", &nnz); // nc
  fscanf(fp,"%d", &nnz); // nnz
  fclose(fp);
  return nnz;
}

void write_ivec (int n, int *x, char *fname) //
{
  FILE *fp = fopen(fname,"w");
  int i;
  assert(fp);
  for (i = 0; i < n; i++)  fprintf(fp, "%d\n", x[i]+1 );
  fclose(fp);
}

void read_ivec (int n, int *x, char *fname) //
{
  FILE *fp = fopen(fname,"r");
  int i;
  assert(fp);
  for (i = 0; i < n; i++)  { fscanf(fp, "%d", x+i ); x[i]--; }
  fclose(fp);
}

/*------------------------------------------
 * randperm
 *------------------------------------------ */

int *randperm(int n) //
{
  int *order = ivec(n);
  int k, nn, takeanumber, temp;
  for (k=0; k<n; k++) order[ k ] = k;
  nn = n;
  for (k=0; k<n; k++) {
    // take a number between 0 and nn-1
    takeanumber = (int) (nn*drand48());
    temp = order[ nn-1 ];
    order[ nn-1 ] = order[ takeanumber ];
    order[ takeanumber ] = temp;
    nn--;
  }
  return order;
}

/*------------------------------------------
 * randomassignment
 *------------------------------------------ */
void randomassignment(int N, int T, int *w, int *d, int *z, int **wp, int **dp, int *ztot) //
{
  int i, t;
  for (i = 0; i < N; i++) {
    t = (int)(T*drand48());
    z[i] = t;
    wp[w[i]][t]++;
    dp[d[i]][t]++;
    ztot[t]++;
  }
}

/*------------------------------------------
 * sample_chain
 *------------------------------------------ */
void sample_chain (int N, int W, int T, double alpha, double beta, int *w, int *d, int *z, int **wp, int **dp, int *ztot, int *order) //
{
  int ii, i, t, word, doc;
  double totprob, draw, cumprob;
  double *probs = dvec(T);
  double wbeta = W*beta;;
  
  for (ii = 0; ii < N; ii++) {

    i = order[ ii ];
    
    t    = z[i];
    word = w[i];
    doc  = d[i];

    ztot[t]--;
    wp[word][t]--;
    dp[doc][t]--;
    totprob = 0;
      
    for (t = 0; t < T; t++) {
      probs[t] = (dp[doc][t] + alpha) * (wp[word][t] + beta) / (ztot[t] + wbeta);
      totprob += probs[t];
    }
    
    draw = drand48()*totprob;
    cumprob = probs[0];
    t = 0;
    
    while (draw>cumprob) {
      t++;
      cumprob += probs[t];
    }
    
    z[i] = t;
    wp[word][t]++;
    dp[doc][t]++;
    ztot[t]++;
  }
 
  free(probs);
 
}

void resample_chain (int N, int W, int T, double alpha, double beta, int *w, int *d, int *z, int **wp, int **dp, int *ztot) //
{
  int i, t, word, doc;
  double totprob, draw, cumprob;
  double *probs = dvec(T);
  double wbeta = W*beta;;
  
  for (i = 0; i < N; i++) {

    t    = z[i];
    word = w[i];
    doc  = d[i];

    dp[doc][t]--;
    totprob = 0;
      
    for (t = 0; t < T; t++) {
      probs[t] = (dp[doc][t] + alpha) * (wp[word][t] + beta) / (ztot[t] + wbeta);
      totprob += probs[t];
    }
    
    draw = drand48()*totprob;
    cumprob = probs[0];
    t = 0;
    
    while (draw>cumprob) {
      t++;
      cumprob += probs[t];
    }
    
    z[i] = t;
    dp[doc][t]++;
  }

  free(probs);
  
}

void loglike (int N, int W, int D, int T, double alpha, double beta, int *w, int *d, int **wp, int **dp, int *ztot, int *dtot) //
{
  int    i, j, t;
  double llike;
  static int init = 0;
  static double **prob_w_given_t;
  static double **prob_t_given_d;
  static double *dtot_;
  double ztot_;

  if (init==0) {
    init = 1;
    prob_w_given_t = dmat(W,T);
    prob_t_given_d = dmat(D,T);
    dtot_ = dvec(D);
    for (j = 0; j < D; j++) dtot_[j] = dtot[j] + T*alpha;
  }
  
  for (t = 0; t < T; t++) {
    ztot_ = ztot[t] + W*beta;
    for (i = 0; i < W; i++) prob_w_given_t[i][t] = (wp[i][t]+beta) / ztot_;
    for (j = 0; j < D; j++) prob_t_given_d[j][t] = (dp[j][t]+alpha)/ dtot_[j];
  }
   
  llike = 0;
  for (i = 0; i < N; i++)
    llike += log(ddot(T, prob_w_given_t[w[i]], prob_t_given_d[d[i]]));
  
  printf(">>> llike = %.6e    ", llike);
  printf("pplex = %.2f\n", exp(-llike/N));
}

void chksum (int n, int T, int **x, int *sumx) //
{
  int i, t, sum;
  for (t = 0; t < T; t++) {
    sum = 0;
    for (i = 0; i < n; i++) sum += x[i][t];
    assert(sum==sumx[t]);
  }
}
