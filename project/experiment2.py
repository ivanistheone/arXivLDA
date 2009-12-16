#!/usr/bin/env python
from mylibs import argparse
from mylibs.ghettologging import gl
from rungen import rungen

import subprocess
import simplejson

import sys,os
import random
import re
import tarfile
import datetime


PROJECT_PATH=os.path.realpath(os.path.join(os.path.dirname(__file__),".."))
logfile = os.path.join(PROJECT_PATH,"project/results/exp2.log")
resultdir = os.path.join(PROJECT_PATH,"project/results/")


#EXPERIMENT 2
#
#    What is the importance of NITER ?
#    i.e. how long for the Gibbs chain to converge to the desired distr.
#

print "EXPERIMENT2 started....."


gl("Building the run queue", logfile)



# prepare the run queue
runs = []
counter = 200
for NITER in [5, 10, 20, 30, 50, 75, 100, 150, 200, 300]:
    gl("Starting iteration with NITER="+str(NITER), logfile)
    counter +=1
    r = rungen(counter, T=20, NITER=NITER, SEED=1)
    runs.append(r)

# run the runs !
for run in runs:

    gl("Starting run "+str(run["name"]), logfile)
    gl("Run description: "+str(run), logfile)

    start_time = datetime.datetime.now()
    gl("start time: "+str(start_time), logfile)


    (out,err)  = subprocess.Popen(run["runcommand"], shell=True, \
                                               stdout=subprocess.PIPE, \
                                               stderr=subprocess.PIPE \
                                               ).communicate()

    end_time = datetime.datetime.now()
    gl("end time: "+str(end_time), logfile)

    duration = (end_time-start_time).seconds
    gl("run duration: "+str(duration)+" seconds", logfile)

    #
    resultfile = resultdir+str(run["name"])+"_results.json"
    result=run
    result["duration"]=duration

    simplejson.dump(result, open(resultfile,"w") )

