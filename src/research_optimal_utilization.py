import itertools
import subprocess
import datetime
import sys
import random

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

def random_trees(N):
    lst = [[j for j in xrange(i+1)] for i in xrange(N+1)]
    while(True):
        yield[random.choice(l) for l in lst]

lower = 0
upper = 0
if sys.argv[1]=="random":
    lower = "random"
    upper = int(sys.argv[2])
else:
    lower = int(sys.argv[1])
    upper = int(sys.argv[2])

if lower=="random":
    fil = open("optimal_utilization_%d_rand.txt"%(upper),"a")
else:
    fil = open("optimal_utilization_%d_%d.txt"%(lower, upper), "a")
for c in [generate_trees(lower, upper), random_trees(upper)][lower=="random"]:
    print c
    fil.write(str(c))
    fil.write("\n")
    tasks = subprocess.Popen(["build/tasks", "-p3", "-s", "leaf", "--optimize", "--direct", "\""+" ".join([str(i) for i in c])+"\""], stdout=subprocess.PIPE)
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
