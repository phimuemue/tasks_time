def test(snapshot):
    print "Testing in python!", snapshot 
    print "We have %d tasks."%snapshot.count_tasks()
    return True
