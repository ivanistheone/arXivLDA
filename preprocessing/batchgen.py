#!/usr/bin/env python
import argparse
import sys, os
import random



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate batch of files" )
    parser.add_argument('-i', '--input', default='alldocs.txt')
    parser.add_argument('-o', '--output')
    parser.add_argument('-p', '--prefix')
    parser.add_argument('-n', '--number', type=int, default=100)
    parser.add_argument('-v', dest='verbose', action='store_true')
    args = parser.parse_args()

    if args.output:
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


    ntotal = len(allfiles)
    if args.number > ntotal:
        args.number = ntotal

    # randomize
    random.shuffle(allfiles)


    outlist = allfiles[0:args.number]

    if args.prefix:

        outlist = [ os.path.join(args.prefix,file) for file in outlist]

    outfile.writelines(outlist)

    # clean up
    outfile.close()




