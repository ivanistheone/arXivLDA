#!/usr/bin/env python
import os
from os.path import join, getsize
import subprocess
import collections 

import sys

from stopwords import STOPWORDS


#import argparse
from string import punctuation


# walk entire data directory
PROJECT_PATH=os.path.realpath(os.path.join(os.path.dirname(__file__),".."))
data_path = os.path.join(PROJECT_PATH, "data/pdf")
text_path = os.path.join(PROJECT_PATH, "data/text")



filename = sys.argv[1]


contents = open(filename, "r").read()

words = collections.defaultdict(long)


doclist1 = (chunk.strip(punctuation).lower() for chunk in contents.split() )
doclist3= ( word for word in doclist1 if word not in STOPWORDS and len(word)>2 )




for word in doclist3:
    words[word] += 1 

top_words = sorted(words.iteritems(), key=lambda(word, count): (-count, word))[:40] 

for word, frequency in top_words:
    print "%s: %d" % (word, frequency)





