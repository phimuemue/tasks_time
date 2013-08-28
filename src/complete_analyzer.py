import sys

infile = sys.argv[1]

curtree = ""

with open(infile) as f:
    optimized = open(infile + ".optimized.txt", "w")
    unoptimized = open(infile + ".unoptimized.txt", "w")
    curfile = unoptimized
    for l in f:
        if l.startswith("0"):
            curtree = l.strip()
            optimized.write(l)
            unoptimized.write(l)
            continue
        if l.startswith("c1 o0"):
            curfile = unoptimized
            continue
        if l.startswith("c1 o1"):
            curfile = optimized
            continue
        curfile.write(l)
        if(curfile == optimized and l.startswith("* ")):
            print curtree
