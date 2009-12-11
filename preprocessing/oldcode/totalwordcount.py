import os
from os.path import join, getsize
import subprocess


# walk entire data directory


PROJECT_PATH=os.path.realpath(os.path.join(os.path.dirname(__file__),".."))

data_path = os.path.join(PROJECT_PATH, "data/pdf")
text_path = os.path.join(PROJECT_PATH, "data/text")


# count total number of files
nfiles = 0

for root, dirs, files in os.walk(data_path):
    nfiles = nfiles + len(files)
print "Startomg pdfto latex conversion on ", str(nfiles), " files"

from datetime import datetime

print "Starting work at:"
print( datetime.now().time() )


filecount = 0
nerrors=0

total_words = 0L

inventory = []


# walk the directories and produce
for root, dirs, files in os.walk(text_path):

    for file in files:
        
        
        filecount+=1
        
        doc = join(root,file)
        (out,err)  = subprocess.Popen(["wc", doc], \
                                       stdout=subprocess.PIPE, \
                                       stderr=subprocess.PIPE \
                                       ).communicate()
        if err:
            print "error on file ", doc
            print err
            nerrors+=1

        if len(out) > 0:
            try:
                tmp = out.lstrip(" ").split(" ")
                sanitized = []
                for item in tmp:
                    if len(item)>0:
                        sanitized.append(item)
                
                wc = long( sanitized[1] )

            except IndexError:
                wc=0
                print "not good for this file...", doc


        # update the running total
        total_words+=wc

        item = {}
        item["number"]=filecount
        item["filename"]=file
        item["fullpath"]=doc
        item["wordcount"]=wc
        inventory.append(item)
        
        if filecount in [ int(nfiles/10)*i for i in [1,2,3,4,5,6,7,8,9] ]:
            print "10% gone by...."


print "total number of words", str(total_words)


import cPickle
fil = open("inventory.dat", "w")
cPickle.dump(inventory, fil )


print "Done work at:"
print( datetime.now().time() )

print "total n of files where somethign on strerr", str(nerrors)





