#!/usr/bin/env python
from mylibs import argparse
import sys,os
import random
import re
import tarfile


PROJECT_PATH=os.path.realpath(os.path.join(os.path.dirname(__file__),".."))
PACK_DIR=os.path.realpath(os.path.join(os.path.dirname(__file__),"packs/"))
RUN_ROOT = os.path.join(PROJECT_PATH, "data/runs/")


def rungen(name,run_path=RUN_ROOT, T=10, NITER=100, SEED=123):
    """ Creates all the necessary setup for 1 experimental run
        returns a dict of values
        { T
          NITER
          SEED
          runcommand
          wpfile
        }
        another"""


    dirname = "run"+str(name)+"_T"+str(T)+"_NITER"+str(NITER)+"_seed"+str(SEED)
    rundir = os.path.join(run_path,dirname)

    if os.path.exists(rundir):
        print "Error -- rundir already exists... exiting"
        return None

    # create dir
    os.mkdir(rundir)
    os.chdir(rundir)

    # prepare files
    packfile = os.path.join(PACK_DIR, sys.platform+"-run-pack.tar")
    tar = tarfile.open(packfile)
    tar.extractall()
    tar.close()


    runcommand = rundir+"/topicmodel "+str(T)+" "+str(NITER)+" "+str(SEED)
    wpfile = rundir+"/wp.txt"
    dpfile = rundir+"/dp.txt"

    # the dict we will be returning
    rundict ={}
    rundict["name"]=name
    rundict["T"]=T
    rundict["NITER"]=NITER
    rundict["SEED"]=SEED
    rundict["rundir"]=rundir
    rundict["runcommand"]=runcommand
    rundict["wpfile"]=wpfile
    rundict["dpfile"]=dpfile


    return rundict




if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Prepares a run" )
    parser.add_argument('--T')
    parser.add_argument('--RUN_ROOT')
    parser.add_argument('--NITER')
    parser.add_argument('--SEED')
    parser.add_argument('-v', dest='verbose', action='store_true')
    args = parser.parse_args()

    kwargs ={}
    if args.RUN_ROOT:
        kwargs["run_root"]=args.RUN_ROOT
    if args.T:
        kwargs["T"]=args.T
    if args.NITER:
        kwargs["NITER"]=args.NITER
    if args.SEED:
        kwargs["SEED"]=args.SEED


    print rungen("tmp",**kwargs)
    #run_path=args.RUN_ROOT, T=args.T, NITER=args.NITER, SEED=args.SEED)




