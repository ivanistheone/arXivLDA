#!/usr/bin/env python
from mylibs import argparse
import sys,os
#import tarfile
import shutil 

try:
    import json as simplejson
except ImportError:
    import simplejson


PROJECT_PATH=os.path.realpath(os.path.join(os.path.dirname(__file__),".."))
PACK_DIR=os.path.realpath(os.path.join(os.path.dirname(__file__),"packs/"))
RUN_ROOT = os.path.join(PROJECT_PATH, "data/runs/")


from numpy import *
from scipy import sparse

#RUN_DIR="../data/runs/run2_first_test/"


def loadsparsemat(filename):
    """ Load a sparse matrix from file """
    indata = open(filename).readlines()
    NROWS=int(indata[0].strip() )       # first line is W
    NCOLS=int(indata[1].strip() )       # T
    nCols =int(indata[2].strip() )
    cols = indata[3:]
    assert( len(cols) == nCols)

    wp = zeros([NROWS,NCOLS])       # undormalized word dopic \beta distr.
    for line in cols:
        if len(line)>0:
            [i,j,d]= line.split()
            wp[int(i)-1,int(j)-1] = d

    return wp
    # old way
    #m = sparse.csc_matrix( (data, (row,col)) , dtype=float)
    #wp=m.toarray()



def get_probs(RUN_DIR):
    """ returns (W, T, D, prob_w_given_t, prob_t_given_d)
        out of a RUN_DIR/wp.txt and RUN_DIR/dp.txt """


    wp = loadsparsemat(RUN_DIR + "wp.txt")
    #wp = wp + 0.01

    W,T = wp.shape

    prob_w_given_t = dot(wp, diag(1/sum(wp,0))  )
    #prob_t_tmp = sum(wp,0)
    #prob_t = prob_t_tmp/ sum(prob_t_tmp)



    # NOW STARTING THE P(T|D) PART OF THE CALCULATION
    ################################################################

    dp = loadsparsemat(RUN_DIR + "dp.txt")
    # add alpha ?
    D=dp.shape[0]

    fac = 1/sum(dp,1)

    print "fac shapre", fac.shape
    print "W, T, D ", W, T, D
    print "dp shape", dp.shape
    print "wp shape", wp.shape


    for t in arange(0,T):
        dp[:,t]=dp[:,t]*fac
        print "t is", t

    prob_t_given_d = dp.transpose()


    #prob_t_given_d.shape
    #prob_w_given_t.shape

    # NOT A GOOD IDEA TOO LARGE BRO
    #prob_w_given_d=dot(prob_w_given_t, prob_t_given_d)


    return (W,T,D, prob_w_given_t, prob_t_given_d)



def get_d_w_cnt(RUN_DIR):  #, D, W):
    """ returns d_w_cnt a shape=[long,3] docword array """

    #   LOAD THE docword.txt file
    ################################################################

    # MUST USE SHARE INSTEAD
    #docword=open(RUN_DIR + "docword.txt")
    docword=open(RUN_DIR + "docword_10000_ALL.txt")

    # how many instances do we have ?
    garbage =docword.readline()
    garbage =docword.readline()

    size = int(docword.readline().strip())

    d_w_cnt = zeros([size,3])
    k=0
    for line in docword:
        if len(line)>0:
            [d,w,cnt]= line.split()
            d_w_cnt[k,:]=[int(d)-1, int(w)-1, int(cnt)]
            k=k+1
    docword.close()

    return d_w_cnt


################################ LOAD VOCAB ###################
#    vocab.append(
#vocab = open(RUN_DIR+'vocab.txt','r').readlines()
#assert(len(vocab)==W)


def calculate_perplexity(d_w_cnt, prob_w_given_t, prob_t_given_d):
    """ guess what it does """
    # compute the perplexity

    N=float( sum(d_w_cnt,0)[2] )
    inner_sum=0.0

    for [d,w,cnt] in d_w_cnt:
        prelog = dot(prob_w_given_t[w,:], prob_t_given_d[:,d])
        inner_sum = inner_sum + cnt*log(prelog)

    pplex = exp(-1*inner_sum/N)
    return pplex



    






if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Updates a set of result.json files with perplexity calulation " )
    parser.add_argument('jsonfiles', nargs='+' )
    parser.add_argument('-v', dest='verbose', action='store_true')
    args = parser.parse_args()

    print "PERPLEXITY CALULATOR"
    print "*"*40

    if not args.jsonfiles:
        print "no inputs given.... exiting "
        sys.exit(-1)

    # load the docword
    # assume they are all the same :)
    dwpath = RUN_ROOT+"share/"
    d_w_cnt = get_d_w_cnt(dwpath)
    

    flag=True


    for result in args.jsonfiles:
        
        # load run dict
        run = simplejson.load(open(result,'r'))

        rundir = run["rundir"]
        if rundir[-1] != "/":
            rundir=rundir+"/"

        (W,T,D, prob_w_given_t, prob_t_given_d) = get_probs(rundir)



        pplex = calculate_perplexity(d_w_cnt, prob_w_given_t, prob_t_given_d)

        print "experiment ", run["name"] 
        print "#ITER=", run["NITER"], "  T= ",T, "pplex=", pplex


        # store it back into place
        run["perplexity"]=pplex
        if flag:
            simplejson.dump(run, open("mytmprun.json","w"))
            flag=False

