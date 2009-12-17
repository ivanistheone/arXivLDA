#!/usr/bin/env python
from mylibs import argparse
from mylibs.ghettologging import gl
from rungen import rungen

import subprocess
import json as simplejson

import sys,os
import random
import re
import tarfile
import datetime

#Number of topics
#================
#
#Run full data set and try to fit diff # of topics
#T = 10, 20, 30, 50, 100, 200
#60 ~ 600Mb RAM
#1000 topics ~ 1G ram
#
#record:
#    max mem usage
#    aprroximate runtime
#calulate:
#    perplexity

PROJECT_PATH=os.path.realpath(os.path.join(os.path.dirname(__file__),".."))
logfile = os.path.join(PROJECT_PATH,"project/results/exp1.log")
resultdir = os.path.join(PROJECT_PATH,"project/results/")


print "EXPERIMENT1 started....."


gl("Building the run queue", logfile)

runs = []
counter = 109
for T in [75, 100, 150, 200, 300]:
    gl("Setting up iteration with T="+str(T), logfile)
    counter +=1
    r = rungen(counter, T=T, NITER=100, SEED=1)
    runs.append(r)


# run the runs !
for run in runs:

    gl("Starting run "+str(run["name"]), logfile)
    gl("Run description: "+str(run), logfile)

    start_time = datetime.datetime.now()
    gl("start time: "+str(start_time), logfile)

    os.chdir(run["rundir"])
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

