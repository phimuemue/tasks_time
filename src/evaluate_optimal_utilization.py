import sys
import re
import itertools

filename = sys.argv[1]
print filename

f = open(filename, "r")

splitter = re.compile("\[(.*)\]:\W+(.*?)\W\((.*)\)")
current_intree = ""
current_stuff = []
count = 0

for l in f:
    if not l.startswith("["): #]
        count = count + 1
        if(len(current_stuff)>1):
            printme3 = False
            printme1 = False
            for (x,y) in itertools.combinations(current_stuff, 2):
                if(x[0] < y[0] and x[1][0] < y[1][0]) or (x[0] > y[0] and x[1][0] > y[1][0]) :
                    printme3 = True
                if(x[0] < y[0] and x[1][2] > y[1][2]) or (x[0] > y[0] and x[1][2] < y[1][2]) :
                    printme1 = True
            printme = printme1 or printme3
            if printme:
                print "Intree " + current_intree,
                print ["","3pl partly suboptimal, "][printme3] + ["","1ps partly suboptimal"][printme1]
                minimums = [x for x in current_stuff if x==min(current_stuff, key=lambda x:x[0])]
                for (m,c) in itertools.product(minimums, current_stuff):
                    if m[1][0] < c[1][0]:
                        print "3pl really suboptimal"
                    if m[1][2] > c[1][2]:
                        print "1ps really suboptimal"
                for i in current_stuff:
                    print i[0], i[1]
        current_intree = l
        current_stuff = []
    else:
        match = splitter.match(l)
        times = [float(x) for x in match.groups()[-1].split("/")]
        current_stuff.append([float(match.groups()[1]), times])

f.close()
