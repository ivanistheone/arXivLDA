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
nerrors = 0

# walk the directories and produce
for root, dirs, files in os.walk(data_path):


    # prepare the output directory
    text_root = root.replace("data/pdf", "data/text")

    if not os.path.exists(text_root):
        os.mkdir(text_root)

    #print text_root

    #for file in files:
    #    print "processing file:", file
    #    print out
    for file in files:

        source = join(root,file)
        destination = join(text_root,file+".txt")
        #print source
        #print destination
        (out,err)  = subprocess.Popen(["pdftotext", source, destination], \
                                       stdout=subprocess.PIPE, \
                                       stderr=subprocess.PIPE \
                                       ).communicate()
        if err:
            print err
            nerrors+=1	
        filecount = filecount+1
        if filecount in [ int(nfiles/10)*i for i in [1,2,3,4,5,6,7,8,9] ]:
            print "10% gone by...."


print "Done work at:"
print( datetime.now().time() )

print "total n of files where somethign on strerr", str(nerrors)

