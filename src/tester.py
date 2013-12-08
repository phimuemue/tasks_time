def test(snapshot):
    print "Testing in python!", snapshot 
    print "We have %d tasks."%snapshot.count_tasks()
    it = snapshot.intree
    print "Tasks: " + ", ".join([str(i) for i in it.get_tasks()])
    print "Predecessors of 0: " + ", ".join([str(i) for i in it.get_predecessors(0)])
    print "Leaves: " + ", ".join([str(i) for i in it.get_leaves()])
    print "Marked: " + ", ".join([str(i) for i in snapshot.marked])
    return True
