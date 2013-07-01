import sys
import itertools

n = int(sys.argv[1])
r = int(sys.argv[2])

cur_max = 0

for c in itertools.combinations_with_replacement([i for i in xrange(1,n+1)], r):
    if sum(c)==n:
        tmp = reduce(lambda x, y: x*y, c, 1)
        if tmp>cur_max:
            cur_max = tmp
        print "%s: %d"%(str(c), tmp)
    

