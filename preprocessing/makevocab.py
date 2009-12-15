#!/usr/bin/env python
import argparse
import sys
import collections
import os
import cPickle
#import random
#import re
#import codecs



def makevocab(wsfile, vocabfile,minoccur=None,maxwords=None,pickleout=None):
    """ Goes through the wordstreamfile generating word counts and decides which
        are the most important words """

    if not os.path.exists('vocab.dat'):
        # prepare default dict where we will store output
        vocab =  collections.defaultdict(long)

        # get first chunk 
        wsfile = open(wsfile, 'r')
        words = wsfile.readlines(10000)

        # iterate over chunks of size 10k
        while words:
            # loop over all files
            for word in words:
                word = word.strip()
                if word[0:3]=="###":    # skip file begin and EOF markers
                    continue
                vocab[word] = vocab[word]+1
            words = wsfile.readlines(10000)
        wsfile.close()

        cPickle.dump(vocab, open("vocab.dat","w"))

    else:   # we have already the vocab
        vocab = cPickle.load(open("vocab.dat","r"))

    
    # prune words that are do not satisfy minoccur
    vocab2 = {} # moving here things that pass the test...
    for word,count in vocab.iteritems():
        if count < minoccur:
            continue
        elif len(word)==3 and count < 10000:
            continue
        elif len(word)==4 and count < 6000:
            continue
        elif len(word)==5 and count < 2000:
            continue
        else:
            vocab2[word]=count
        
    # sort by freq
    wordtuples=sorted(vocab2.items(),key=lambda(word,count): (-count,word) )

    # cur off only maxwords worth 
    if maxwords:
        finalvocab=wordtuples[0:maxwords]
    else:
        finalvocab=wordtuples

    # write in order
    fout = open(vocabfile,"w")
    for word, count in finalvocab:
        #fout.write(word+" "+str(count)+"\n")
        fout.write(word+"\n")
    fout.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Produces a vocab.txt file from a word stream" )
    parser.add_argument('-i', '--input',  help="wordsteam file")
    parser.add_argument('-o', '--output', help="name of vocab file",default="vocab.txt")
    parser.add_argument('--minoccur',  help="minimum times word must occur", type=int, default=45)
    parser.add_argument('--maxwords',  help="max number of words in vocab", type=int)
    parser.add_argument('-v', dest='verbose', action='store_true')
    args = parser.parse_args()

    print "VOCAB extractor"
    if not args.input:
        print "no input specified,.... exiting "
        sys.exit(1)

    # read the lines
    files = open(args.input,'r').readlines()

    makevocab(args.input, args.output, minoccur=args.minoccur, maxwords=args.maxwords)


