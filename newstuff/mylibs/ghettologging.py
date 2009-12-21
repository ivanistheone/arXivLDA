
def gl(msg,logfile,   also_print=True):
    """ appends any string to the logfile logfile
        adds a newline and if also_print is true prints to STDOUT"""

    lf = open(logfile,"a")
    lf.write(msg+"\n")
    lf.close()
    if also_print:
        print msg


