#!/usr/bin/env python
import argparse
import sys
import random



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate batch of files" )
    parser.add_argument('-i', '--input', default='alldocs.txt')
    parser.add_argument('-o', '--output')
    parser.add_argument('-n', '--number', type=int, default=100)
    parser.add_argument('-v', dest='verbose', action='store_true')
    args = parser.parse_args()

    print "BATCH GENERATOR :::::::"
    if args.verbose:
        print "number of files ", args.number
        print "randomly selected from ", args.input
        print "written to ", args.output

    allfiles = open(args.input,'r').readlines()

    if args.output:
        outfile = open(args.output,'w')
    else:
        outfile = sys.stdout

    # make sure there are no blank lines
    allfiles = [file for file in allfiles if len(file.strip() )>0]

    #ntotal = len(allfiles)

    # randomize
    random.shuffle(allfiles)

    outfile.writelines(allfiles[0:args.number])

    # clean up
    outfile.close()




