from random import randint, choice, sample
from copy import deepcopy

from distributions.distributions import ExponentialDistribution

def randomTreeEdges(n):
    """Returns a random tree with n vertices"""
    return [(Task(i),Task(randint(0,i-1))) for i in xrange(1,n)]

class Task(object):
    """A simple class representing one single task.
    Basically, this is only data encapsulation.
    However, note that probability distribution of duration
    is not handled within this class, but in distributions
    module.
    Task class only stores the elapsed time so it can be fed
    to the distributions module that computes some stuff with
    it."""
    def __init__(self, id):
        self.id = id
        self._elapsed = 0
    def __repr__(self):
        return "%s"%str(self.id)
    @property
    def elapsed(self):
        return self.elapsed_time
    @elapsed.setter
    def set_elapsed(self, e):
        self._elapsed = e
    # (In)equality is only given by id
    def __eq__(self, other):
        return self.id == other.id
    def __ne__(self, other):
        return self.id != other.id

class Intree(object):
    """A collection of tasks forming a dependency intree.
    Only edges are stored, vertices are then obtained implicitly."""
    def __init__(self, edges):
        self.edges = edges
    def __repr__(self):
        return str(self.edges)
    def get_in_degree(self, v):
        """Returns the in-degree of vertex v.
        This corresponds to the number of tasks to be completet
        until v is ready to be scheduled."""
        result = 0
        for (a,b) in self.edges:
            if b==v:
                result = result + 1
        return result
    def get_vertices(self):
        """Returns a list of vertices."""
        result = [Task(0)]
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
        return str(self.chains())


class Snapshot(object):
    """This class represents an intree (i.e. a dependency graph of tasks),
    the currently scheduled tasks (these are marked), and can compute 
    possible successors."""
    def __init__(self, intree, marked):
        self.intree = intree
        self.marked = marked
        self.successors = None
        self.expected = None
    def __repr__(self):
        return str(self.intree.chains()) + " | " + str(self.marked)
    def get_successors(self):
        """Returns all possible successive constellations (dependency intrees
        and marked threads). Return value is cached (use of that not yet determined)."""
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
        """Computes a snapshot-dag."""
        for suc in self.get_successors():
            print "Going to child: %s"%str(suc)
            suc.compile_snapshot_dag()
    def expected_runtime(self, printme=False):
        """Computes expected runtime originating at this configuration."""
        if self.expected != None:
            return self.expected
        if len(self.get_successors())==0:
            return 1.0
        result = 0.0
        for suc in self.get_successors():
            additum = suc.expected_runtime() + 1./len(self.marked)
            if printme:
                print "Considering successor %s : %f"%(str(suc), suc.expected_runtime())
                print "#sucs: %d"%len(self.get_successors())
                print "Additum: " + str(additum)
            result = result + additum
        result = result/len(self.get_successors())
        self.expected = result
        return result



num_tasks = 8
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

