import itertools
import sys
from math import log
from math import ceil
import re
import subprocess

def generate_trees(M,N):
    for n in xrange(M,N+1):
        print "Beginning with %d tasks"%n
        lst = [[j for j in xrange(i+1)] for i in xrange(n)]
        for comb in itertools.product(*lst):
            useful = True
            if comb[0]!=0:
                useful = False
            if useful:
                for i in xrange(len(comb)-1):
                    if comb[i]>comb[i+1]:
                        useful = False
                        break
            if not useful:
                continue
            good = True
            for i in xrange(n):
                if comb[i]>i:
                    good = False
            if good:
                yield comb

def generate_growing_seq_sum_n_m(n, m, maxval, depth=1):
    if n<m:
        return    
    if n==m:
        yield [1] * m
        return
    #print " "*depth, n, m, maxval
    for i in xrange(1, min(n, maxval)+1):
        for tmp in generate_growing_seq_sum_n_m(n-i, m-1, i, depth+1):
            yield [i] + tmp

a = 15
b = 6

for i in generate_growing_seq_sum_n_m(a, b, a):
    print i
exit()

def generate_hashes_clever(n):
    if n==1:
        return "1"
    result = "["
    result = result + "]"
    for i in xrange(1, n): # we can have 1,2,3,...,(n-1) precursors
        for child_degrees in generate_trees2(i):
            print [1+x for x in child_degrees]
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

class Intree(object):
    """A collection of tasks forming a dependency intree.
    Only edges are stored, vertices are then obtained implicitly."""
    def __init__(self, edges, real_edges = False):
        if real_edges:
            self.edges = edges
            return
        self.edges = []
        for i in enumerate(edges):
            self.edges.append((i[0]+1,i[1]))
    def __repr__(self):
        chains = []
        for v in self.edges:
            chains.append(self.get_chain(v[0]))
        realchains = []
        for c in chains:
            add = True
            for e in self.edges:
                if e[1]==c[0]:
                    add = False
            if add:
                realchains.append(c)
        return str(realchains)
    def get_in_degree(self, v):
        """Returns the in-degree of vertex v.
        This corresponds to the number of tasks to be completet
        until v is ready to be scheduled."""
        result = 0
        for (a,b) in self.edges:
            if b==v:
                result = result + 1
        return result
    def get_predecessors(self, v):
        """Returns v's predecessors"""
        result = []
        for (a,b) in self.edges:
            if b==v:
                result.append(a)
        return result
    def get_vertices(self):
        """Returns a list of vertices."""
        result = [0]
        for (a,b) in self.edges:
            if a not in result:
                result.append(a)
        return result
    def get_random_leave(self): # this should be in the scheduler
        """Obtains a random leave, i.e. a randomly chosen task
        that is ready to be processed (i.e. that has no
        predecessors)."""
        possible = filter(lambda x: self.get_in_degree(x)==0, [v for v in self.get_vertices()])
        return choice(possible)
    def remove_vertex(self, v):
        """Removes all edges that go out from vertex v.
        Note that this might remove additional vertices x if x is only
        reachable via v -- which should not be a problem in our application."""
        self.edges = filter(lambda x: x[0]!=v, self.edges)
    def get_edge_from(self, v):
        """Returns the edge from vertex v to another vertex."""
        for (a,b) in self.edges:
            if a==v:
                return (a,b)
        return None
    def get_chain(self, v):
        """Gets the dependency chain starting at vertex/task v going to task 0."""
        if v == 0:
            return [0]
        result = []
        result.append(v)
        cur_edge = self.get_edge_from(v)
        while cur_edge[1] != 0:
            result.append(cur_edge[1])
            cur_edge = self.get_edge_from(cur_edge[1])
        return result + [0]
    def get_longest_chain(self):
        """Returns the longest dependency chain."""
        longest = 0
        best = [0]
        for v in self.get_vertices():
            tmp = self.get_chain(v)
            if len(tmp)>longest:
                longest = len(tmp)
                best = tmp
        return best
    def chains(self):
        """Returns a list of dependency chains."""
        chains = [self.get_chain(v) for v in self.get_vertices()]
        chains.sort(key = lambda x: -len(x))
        # clean chains
        useful_chains = []
        for (i, chain) in enumerate(chains):
            if not any([chain[0] in chains[j] for j in xrange(i)]):
                useful_chains.append(chain)
        return useful_chains
    def nice_string(self):
        # TODO: actually create a cool ascii-graph
        return str(self.chains())
    def as_lisp(self, v=0):
        result = 1
        predecessors = self.get_predecessors(v)
        tmp = []
        for p in predecessors:
            tmp.append(self.as_lisp(p))
        if tmp!=[]:
            result = [1]
            result.append(tmp)
        return result
    def get_canonical(self):
        tmp = self.as_lisp()
        def rec_sort(l):
            if type(l)==int:
                return l
            return sorted(rec_sort(x) for x in l)
        return rec_sort(tmp)
    def __hash__(self):
        tmp = self.get_canonical()
        result = 0l
        def canonical_to_hash(l):
            if type(l)==int:
                return 0b11l
            result = 0b10l
            for i in l:
                tmp = canonical_to_hash(i)
                result = result * 2l**long(ceil(log(tmp,2)))
                result = result + tmp
            result = result * 4l
            result = result + 0b01l
            return result
        return canonical_to_hash(tmp)
    def __eq__(self, other):
        return self.get_canonical() == other.get_canonical()

def count_subtrees(it, pool):
    if (it) in pool:
        return 0
    result = 1
    for e in it.edges:
        if it.get_in_degree(e[0]) == 0:
            tmp_edges = it.edges[:]
            tmp_edges.remove(e)
            t = Intree(tmp_edges, True)
            result = result + count_subtrees(t, pool)
    pool[(it)] = result
    return result


tpool = {}
for i in xrange(1,8):
    print "Beginning with %d tasks:" % (i+1)
    curmax = 0
    for tmp in generate_trees2(i):
        it = Intree(tmp)
        if it not in tpool:
            # second arg necessary!!!
            val = count_subtrees(it, {})
            tpool[it] = val
            if val >= curmax:
                print it, val
                curmax = val
        sys.stdout.flush()
