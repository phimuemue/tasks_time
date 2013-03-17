from random import randint, choice, sample
from copy import deepcopy

def randomTreeEdges(n):
    """Returns a random tree with n vertices"""
    return [(Task(i),Task(randint(0,i-1))) for i in xrange(1,n)]

class Task(object):
    def __init__(self, id):
        self.id = id
    def __repr__(self):
        return "{%s}"%str(self.id)
    def __eq__(self, other):
        return self.id == other.id
    def __ne__(self, other):
        return self.id != other.id

class Intree(object):
    def __init__(self, edges):
        self.edges = edges
    def __repr__(self):
        return str(self.edges)
    def get_in_degree(self, v):
        result = 0
        for (a,b) in self.edges:
            if b==v:
                result = result + 1
        return result
    def get_vertices(self):
        result = [Task(0)]
        for (a,b) in self.edges:
            if a not in result:
                result.append(a)
        return result
    def get_random_leave(self):
        possible = filter(lambda x: self.get_in_degree(x)==0, [v for v in self.get_vertices()])
        return choice(possible)
    def remove_vertex(self, v):
        self.edges = filter(lambda x: x[0]!=v, self.edges)
    def get_edge_from(self, v):
        for (a,b) in self.edges:
            if a==v:
                return (a,b)
        return None
    def get_chain(self, v):
        if v.id == 0:
            return [Task(0)]
        result = []
        result.append(v)
        cur_edge = self.get_edge_from(v)
        while cur_edge[1] != Task(0):
            result.append(cur_edge[1])
            cur_edge = self.get_edge_from(cur_edge[1])
        return result + [Task(0)]
    def get_longest_chain(self):
        longest = 0
        best = [0]
        for v in self.get_vertices():
            tmp = self.get_chain(v)
            if len(tmp)>longest:
                longest = len(tmp)
                best = tmp
        return best
    def chains(self):
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


class Snapshot(object):
    def __init__(self, intree, marked):
        self.intree = intree
        self.marked = marked
        self.successors = None
        self.expected = None
    def __repr__(self):
        return str(self.intree.chains()) + " | " + str(self.marked)
    def get_successors(self):
        # TODO: Implement different scheduling stratgies, foremost HLF
        if len(self.intree.get_vertices())==1:
            return []
        if self.successors == None:
            self.successors = []
            for v in self.marked:
                found = False
                # construct resulting tree after removing v
                newtree = deepcopy(self.intree)
                newtree.remove_vertex(v)
                for nexttask in newtree.get_vertices():
                    if newtree.get_in_degree(nexttask)==0 and nexttask not in self.marked:
                        found = True 
                        newmarked = self.marked[:]
                        newmarked.remove(v)
                        newmarked.append(nexttask)
                        self.successors.append(deepcopy(self.intree))
                        self.successors[-1].remove_vertex(v)
                        self.successors[-1] = Snapshot(self.successors[-1], newmarked)
                if not found:
                    newmarked = self.marked[:]
                    newmarked.remove(v)
                    self.successors.append(deepcopy(self.intree))
                    self.successors[-1].remove_vertex(v)
                    self.successors[-1] = Snapshot(self.successors[-1], newmarked)
        return self.successors
    def compile_snapshot_dag(self):
        for suc in self.get_successors():
            print "Going to child: %s"%str(suc)
            suc.compile_snapshot_dag()
    def expected_runtime(self, printme=False):
        if self.expected != None:
            return self.expected
        if len(self.get_successors())==0:
            return 1.0
        result = 0.0
        for suc in self.get_successors():
            additum = suc.expected_runtime()+ 1./len(self.marked)
            if printme:
                print "Considering successor %s : %f"%(str(suc), suc.expected_runtime())
                print "#sucs: %d"%len(self.get_successors())
                print "Additum: " + str(additum)
            result = result + additum
        result = result/len(self.get_successors())
        self.expected = result
        return result



num_tasks = 5
num_procs = 2

tasks = Intree(randomTreeEdges(num_tasks))
print "Tasks and in-degrees:"
for v in tasks.get_vertices():
    print (v, tasks.get_in_degree(v))
print "Done."

print "Tasks: %s"%str(tasks)
for t in tasks.get_vertices():
    print (t, tasks.get_chain(t))
print tasks.get_longest_chain()

print "Task chains:" + str(tasks.chains())

all_available_tasks = [v for v in tasks.get_vertices() if tasks.get_in_degree(v)==0]

s = Snapshot(tasks, sample(all_available_tasks, min(num_procs, len(all_available_tasks))))
print "Initial state: %s"%str(s)
for i in s.get_successors():
    print i
print "Expected run time: %f"%s.expected_runtime()

