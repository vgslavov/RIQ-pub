#!/usr/bin/env python

'''
Install the lastest version of rdflib:
apt-get install python-rdflib
pip install rdflib --upgrade

Pre-process original data to fix any formatting errors (see docs/)
'''

from __future__ import print_function
from rdflib import Graph, ConjunctiveGraph
from rdflib.plugins.parsers import ntriples
import argparse
import re
import subprocess

def main():
    parser = argparse.ArgumentParser(description='convert n-quad format rdf file to n3 format')
    parser.add_argument('-i', '--infile', required=True, help='input nquad rdf file')
    parser.add_argument('-o', '--outfile', required=True, help='output n3 rdf file')
    args = parser.parse_args()
    outf = open(args.outfile, 'w')

    prevctx = ''
    nderr = 0
    neerr = 0
    nperr = 0
    nserr = 0
    nqtrip = 0
    with open(args.infile) as inf:
        for line in inf:
            #print(line)
            g = ConjunctiveGraph()
            try:
                g = g.parse(data=line, format='nquads')
            except ntriples.ParseError as e:
                print("parsing error: %s" % e)
                nperr += 1
            except UnicodeDecodeError as e:
                print("decoding error: %s" % e)
                nderr += 1
                nqtrip += 1
                continue

            for ctx in g.store.contexts():
                try:
                    strctx = ctx.identifier.encode('utf-8')
                    # doesn't work with unicode
                    #strctx = str(ctx.identifier)
                except UnicodeEncodeError as e:
                    print("encoding error: %s" % e)
                    print(strctx)
                    neerr += 1

            if strctx != prevctx:
                print("# ctx: %s" % strctx, file=outf)

            try:
                nt = ctx.serialize(format='nt')
            except Exception as e:
                print("serialization error: %s" % e)
                nqtrip += 1
                nserr += 1
                prevctx = strctx
                strctx = ''
                continue

            print("%s" % nt.strip(), file=outf)
            nqtrip += 1
            prevctx = strctx
            strctx = ''

    outf.close()

    print("parsing errors: %d" % nperr)
    print("decoding errors: %d (skipped)" % nderr)
    print("encoding errors: %d" % neerr)
    print("serialization errors: %d (skipped)" % nserr)

    nctx = 0
    nctx = grepc_re(args.outfile, '# ctx:')
    # slower?
    #nctx = grepc_shell(args.outfile, '# ctx:')
    print("contexts: %d" % nctx)

    print("nquad triples: %d" % nqtrip)

    nttrip = 0
    nttrip = file_len(args.outfile)
    nttrip -= nctx
    print("nt triples: %d" % nttrip)

    if nqtrip == nttrip:
        print("success: # of triples matches")
    else:
        print("failure: # of triples differs")

# http://code.activestate.com/recipes/577069-access-grep-from-python/
def grepc_shell(filename, pattern):
    process = subprocess.Popen(['grep', '-c', pattern, filename], stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    #return stdout, stderr
    return stdout

# http://code.activestate.com/recipes/577069-access-grep-from-python/
def grepc_re(filename, pattern):
    count = 0
    grepper = re.compile(pattern)
    with open(filename) as f:
        for line in f:
            if grepper.search(line):
                count += 1
    return count

# http://stackoverflow.com/questions/845058/how-to-get-line-count-cheaply-in-python
def file_len(filename):
    with open(filename) as f:
        for i, l in enumerate(f, 1):
            pass
    return int(i)

if __name__ == "__main__":
    main()

    #import sys
    #print(file_len(sys.argv[1]))
    #grepc_re(sys.argv[1], 'ctx:')
    #grepc_shell(sys.argv[1], 'ctx:')
