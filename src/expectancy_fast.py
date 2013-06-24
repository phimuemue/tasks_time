from fractions import Fraction
from random import randint

def generate_seq(depth, length):
    if depth==0:
        for n in xrange(length):
            yield n*int(pow(2, n-1))
        return
    summ = 0
    for n in generate_seq(depth-1, length):
        summ = summ + n
        yield summ

def get_seq_elem(depth, n):
    tmp = list(generate_seq(depth, n+1))
    return tmp[-1]

def exp_test(levels):
    if all([x==1 for x in levels]):
        return Fraction(len(levels))
    newlevels = [1]
    threshold_found = False
    for i in levels[1:]:
        if threshold_found:
            newlevels.append(i)
        else:
            if i > 1:
                newlevels.append(i-1)
                threshold_found = True
            else:
                newlevels.append(1)
    return Fraction((levels[0]),2) + Fraction(1,2) * ( exp_test(levels[1:]) + exp_test(newlevels) )

#print referencelist, ": ", ref

l = 4
pos = 2
referencelist = [1 for _ in xrange(l)]
#referencelist = [randint(1,4) for _ in xrange(l)]
referencelist[pos] = 1
ref = exp_test(referencelist)

for i in xrange(1,8):
    for j in xrange(1,2):
        lst = [2] * i + [1] * j
        print "[%d, %d] %s"%(i, j, str(exp_test(lst)))
        lst2 = [1,1] + lst
        print "[%d, %d]' %s"%(i, j, str(exp_test(lst2)))
        print exp_test(lst2) - exp_test(lst)

print(exp_test([1,2,1]))
exit()

for i in xrange(1, 7):
    lst = referencelist[:]
    lst[pos] = i
    print "%s: %s [%s]"%(str(lst).ljust(len(str(referencelist))+4), str(exp_test(lst)).ljust(10), str(exp_test(lst)-ref))
    



# for i in xrange(1,10):
#     print exp_test([i for _ in xrange(2)])
# 
for i in xrange(1, 6):
        lst = referencelist[:]
        lst[randint(0,len(lst)-1)] = randint(1,5)
        #lst[randint(0,len(lst)-1)] = randint(1,10)
        if(randint(1,2)==2):
            lst[0] = randint(1,5)
        nr = [float(get_seq_elem(d, n-1))/pow(2,n+(d-1)) for (d, n) in enumerate(lst)]
        print nr
        # compute result experimental
        result = exp_test(lst)
        # test: formula correct?
        r = float(len(lst))
        test = r
        for nri in nr:
            test = test + nri
        print "%s: %s\t (%f / %f) \t %f %f"%(str(lst).ljust(10), str(result).ljust(10), float(result), test,  result-ref, (result-ref)*pow(2.,2))
