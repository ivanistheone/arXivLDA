#!/usr/bin/env python
import argparse
import sys
import random



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Generate batch of files" )
    parser.add_argument('-i', '--input')
    parser.add_argument('-o', '--output')
    parser.add_argument('-v', dest='verbose', action='store_true')
    args = parser.parse_args()

    print "DUP REMOVER :::::::"
    if not args.input:
        print "no input specified,.... exiting "
        sys.exit(1)

    # read the lines
    all = open(args.input,'r').readlines()

    if args.output:
        outfile = open(args.output,'w')
    else:
        outfile = sys.stdout


    import re
    arXid=re.compile(r"\d{4}/(?P<id>\d{7})v(?P<ver>\d+)\.pdf\.txt")


    tmplist = []    # list of dicts
                    # { filename
                    #   id
                    #   ver

    for line in all:
        m = arXid.search(line)
        if not m:
            print "error didn't match on:", line
        d = m.groupdict()
        d["line"]=line
        d["ver"]=int(d["ver"])
        tmplist.append(d)


    finallist = []  # subset of tmplist that we want
    for current in tmplist:

        cid=current["id"]
        cver=current["ver"]
        flag=True           # by default ADD TO LIST

        # see if we should add current to the list
        for other in tmplist:
            if cid == other["id"] and cver<other["ver"]:
                flag=False

        if flag:
            finallist.append(current["line"])


    # write out
    outfile.writelines(finallist)


    # clean up
    outfile.close()




