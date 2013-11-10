import itertools
import sys
from math import log
from math import ceil
import re
import subprocess
import collections
from collections import deque

def generate_growing_seq_sum_n_m(n, m, maxval, depth=1):
    if n<m:
        return    
    if n==m:
        yield [1] * m
        return
    for i in xrange(1, min(n, maxval)+1):
        for tmp in generate_growing_seq_sum_n_m(n-i, m-1, i, depth+1):
            yield [i] + tmp

def generate_hashes_clever2(n):
    assert(n>0)
    if n==1:
        yield "[]"
        return
    if n==2:
        yield "[[]]"
        return
    for i in xrange(1, n): # we can have 1,2,3,...,(n-1) precursors
        for child_degrees in generate_growing_seq_sum_n_m(n-1, i, n-1):
            degs = [0] * (n)
            for v in child_degrees:
                degs[v] = degs[v] + 1
            children = []
            for (p, x) in enumerate(degs):
                if x > 0:
                    new = list(generate_hashes_clever2(p))
                    for xx in xrange(x):
                        children.append(new)
            children.reverse()
            for comb in itertools.product(*children):
                yield "[%s]"%("".join(comb))
    return 

def generate_hashes_clever(n, exclude_trivials=False):
    assert(n>0)
    # base cases seem to significantly speed up the generation
    base_cases = [
        [],
        ["[]"],
        ["[[]]"],
        ["[[[]]]","[[][]]"],
        ["[[[[]]]]", "[[[][]]]", "[[[]][]]", "[[][][]]"],
    ]
    if n < len(base_cases):
        for i in base_cases[n]:
            yield i
        return
    for i in xrange(1 if not exclude_trivials else 2, n): # we can have 1,2,3,...,(n-1) precursors
        for child_degrees in generate_growing_seq_sum_n_m(n-1, i, n-1):
            degs = [0] * (n)
            for v in child_degrees:
                degs[v] = degs[v] + 1
            children = []
            for (p, x) in enumerate(degs):
                if x > 0:
                    new = list(generate_hashes_clever(p))
                    children.append(list(itertools.combinations_with_replacement(new, x)))
            children.reverse()
            for comb in itertools.product(*children):
                tmp = ["".join(x) for x in comb]
                yield "[%s]"%("".join(tmp))
    return 

def list_from_hash(h, depth=0, start=0):
    result = []
    counter = 0
    dq = deque()
    dq.append((0, 0, len(h)))
    startidx = 1
    sucnum = 0
    while(len(dq) > 0):
        parent, l, r = dq.popleft()
        if sucnum>0:
            result.append(parent)
        for i in xrange(l+1, r-1):
            c = h[i]
            if c == "[":
                if counter==depth:
                    startidx = i
                counter = counter + 1
            if c == "]":
                counter = counter - 1
                if counter == 0:
                    dq.append((sucnum, startidx, i+1))
        sucnum = sucnum + 1
    news = len(result)
    return result

def generate_trees2(n):
    def generate_trees2_int(n, t, minidx, maxidx):
        if n==0:
            yield []
            return
        for first in xrange(minidx, maxidx+1):
            for comb in generate_trees2_int(n-1, t, first, maxidx+1):
                yield [first] + comb
    for i in generate_trees2_int(n, n, 0, 0):
        yield i

def generate_trees(n, exclude_trivials=False):
    for i in generate_hashes_clever(n, exclude_trivials):
        yield list_from_hash(i)

def count_leaves(l):
    nl = {a for a in l}
    return len(l)+1-len(nl)

def generate_parallel_chains(n):
    def gen_chains(n, r, minidx):
        if r==1:
            if n>=minidx:
                yield [n]
            return
        for i in xrange(minidx, n):
            for pc in gen_chains(n-i, r-1, i):
                yield [i] + pc
    for r in xrange(1, n+1):
        for pc in gen_chains(n, r, 1):
            tree = list(xrange(n))
            s = 0
            for cl in pc[:-1]:
                s = s + cl
                tree[s] = 0
            yield tree

with open("../database/parallel_chains.txt", "w") as f:
    for n in xrange(28,31):
        for i in generate_parallel_chains(n):
            for num in i:
                f.write("%d "%num)
            f.write("\n")


for n in xrange(1,16):
    print "Working with %d tasks."%n
    maxsubt = 0
    for i in generate_trees(n, False):
        if s(i) > 0:
            for num in i:
                f.write("%d "%num)
            f.write("\n")




# for i in xrange(6):
#     for pc in generate_parallel_chains(i):
#         print pc

# for n in xrange(1,18):
#     print "Working with %d tasks."%n
#     with open("../database/%d_all.txt"%n, "w") as f:
#         for i in generate_trees(n, False):
#             if count_leaves(i) > 0:
#                 for num in i:
#                     f.write("%d "%num)
#                 f.write("\n")



