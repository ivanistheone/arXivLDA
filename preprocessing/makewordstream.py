#!/usr/bin/env python
import argparse
import sys
import random
import re
import codecs

from stopwords import STOPWORDS



LIGATURES = {   u"\u00DF": "fs",
                u"\u00C6": "AE",
                u"\u00E6":"ae",
                u"\u0152":"OE",
                u"\u0153":"oe",
                u"\uFB00":"ff",
                u"\uFB01":"fi",
                u"\uFB02":"fl",
                u"\uFB03":"ffi",
                u"\uFB04":"ffl",
                u"\uFB05":"ft", }
# see wikipedia: http://en.wikipedia.org/wiki/Typographic_ligature


def makewordstream(files_list, write_to):
    """ This funciton goes through each of the text files in files_list
        reads each file and spits out a long list """



    # setup the output
    fout = open(write_to,"w")

    # loop over all files
    for file in files_list:
        fout.write("### "+file)

        text = codecs.open(file.strip(),'r','utf-8').read()

        # to lowercase
        text=text.lower()

        # expand ligatures to ordinary letters
        for ligature,equiv in LIGATURES.items():
            text = text.replace(ligature,equiv)

        # convert to ASCII BYTE STRING
        simpletext = text.encode('ascii', 'ignore') 


        # setup the regex for words that consist of letters ONLY
        word=re.compile(r"[a-z]+")

        for m in word.finditer(simpletext):
            # we are on word w
            
            w = m.group(0)

            # skip stopwords
            if w in STOPWORDS:
                continue

            # skip two letter words
            if len(w)<3:
                continue

            fout.write(w+"\n")

        fout.write("###EOF###\n")
        # done loop
    



    fout.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Produces a long stream of words" )
    parser.add_argument('-i', '--input')
    parser.add_argument('-o', '--output')
    parser.add_argument('-v', dest='verbose', action='store_true')
    args = parser.parse_args()

    print "Word steam generator"
    if not args.input:
        print "no input specified,.... exiting "
        sys.exit(1)
    if not args.output:
        print "no output specified,.... exiting "
        sys.exit(1)

    # read the lines
    files = open(args.input,'r').readlines()

    makewordstream(files, write_to=args.output)




