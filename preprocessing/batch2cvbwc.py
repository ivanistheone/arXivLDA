#!/usr/bin/env python
import os
from os.path import join, getsize
import subprocess
import collections 
import sys
import cPickle

from stopwords import STOPWORDS

#import argparse
from string import punctuation



PROJECT_PATH=os.path.realpath(os.path.join(os.path.dirname(__file__),".."))
data_path = os.path.join(PROJECT_PATH, "data/pdf")
text_path = os.path.join(PROJECT_PATH, "data/text")
wordcounts_path = os.path.join(PROJECT_PATH, "data/wordcounts")

# count total number of files
nfiles = 0

for root, dirs, files in os.walk(data_path):
    nfiles = nfiles + len(files)
print "Startomg pdfto latex conversion on ", str(nfiles), " files"

# START
from datetime import datetime
print "Starting work at:"
print( datetime.now().time() )



filecount = 0


all_words = collections.defaultdict(long)

inventory = []


# walk the directories and produce 
for root, dirs, files in os.walk(text_path):

    for file in files:
        
        filecount +=1
        

        doc = join(root,file)
        contents = open(doc, "r").read()

        doc_words = collections.defaultdict(long)

        doclist1 = (chunk.strip(punctuation).lower() for chunk in contents.split() )
        doclist3= ( word for word in doclist1 if word not in STOPWORDS and len(word)>2 )


        # update the doc_dict
        for word in doclist3:
            doc_words[word] += 1 


        #update the all_dict

        for word in doc_words.iterkeys():
            all_words[word]+=doc_words[word]


        # prepare saved data 
        item = {}
        item["number"]=filecount
        item["filename"]=file
        item["fullpath"]=doc
        item["wordcounts"]=doc_words


        # prepare to write to outfilename
        outfilename = os.path.join(wordcounts_path,file+".wc")
        if os.path.exists(outfilename):
            print "overwriting filename", outfilename
        outfile = open( outfilename, "w" )
        cPickle.dump(item, outfile)
        outfile.close()
        
        
        #inventory.append(item)
        
        if filecount in [ int(nfiles/10)*i for i in [1,2,3,4,5,6,7,8,9] ]:
            print "10% gone by...."





fil = open("all_words.dat", "w")
cPickle.dump(all_words, fil )
fil.close()




print "Done work at:"
print( datetime.now().time() )



top_words = sorted(all_words.iteritems(), key=lambda(word, count): (-count, word))[:40] 

for word, frequency in top_words:
    print "%s: %d" % (word, frequency)







