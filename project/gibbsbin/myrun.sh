#!/bin/bash

#----------------------------------------------------
# file:    myrun.sh
# purpose: run topic model end-to-end
# usage:   run.sh
# inputs:  docs.txt
# outputs: topics.txt, topicsindocs.txt
# version: 1.0
# author:  David Newman, newman@uci.edu
# date:    11/19/08
#----------------------------------------------------


### create wordstream.txt
./Makewordstream.pl < docs.txt > wordstream.txt

### create vocab.txt
./Makevocab.pl < wordstream.txt > vocab.txt

### create docword.txt
./Makedocword.pl < wordstream.txt > docword.txt

### run topic model (T=10 topics, iter=200 iterations), create wp.txt, dp.txt, z.txt
./topicmodel 10 200

### print topics.txt
./topics > topics.txt
#matlab -nojvm < topics.m

### print topicsindocs.txt
./topicsindocs > topicsindocs.txt
#matlab -nojvm < topicsindocs.m

date
ls -lrt *.txt
