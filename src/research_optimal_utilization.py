import itertools
import subprocess
import datetime
import sys

def generate_trees(M,N):
    for n in xrange(M,N+1):
        print "Beginning with %d tasks"%n
        lst = [[j for j in xrange(i+1)] for i in xrange(n)]
        for comb in itertools.product(*lst):
            good = True
            for i in xrange(n):
                if comb[i]>i:
                    good = False
            if good:
                yield comb

lower = int(sys.argv[1])
upper = int(sys.argv[2])

fil = open("optimal_utilization_%d_%d.txt"%(lower, upper), "a")
for c in generate_trees(lower, upper):
    print c
    fil.write(str(c))
    fil.write("\n")
    tasks = subprocess.Popen(["build/tasks", "-p3", "-s", "leaf", "--direct", "\""+" ".join([str(i) for i in c])+"\""], stdout=subprocess.PIPE)
    tasks.wait()
    output = tasks.communicate()[0]
    printing = False
    for l in output.splitlines():
        if l.startswith("Best"):
            printing = False
        if printing:
            fil.write(l)
            fil.write("\n")
        if l.startswith("Computing"):
            printing = True
fil.close()
