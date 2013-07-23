import glob

degrees = [x for x in xrange(20)]

max_raw = {x:0 for x in degrees}
max_opt = {x:0 for x in degrees}
sum_raw = {x:0 for x in degrees}
sum_opt = {x:0 for x in degrees}

for d in degrees:
    print ""
    print "Degree %d"%d
    counter = 0
    max_raw_trees = []
    max_opt_trees = []
    for f in glob.glob("subtree_database/subtree_database_%d.txt*.result"%d):
        curfile = open(f, "r")
        for line in curfile:
            counter = counter + 1
            parts = line.strip().split(" | ")
            parts[1] = int(parts[1])
            parts[2] = int(parts[2])
            if parts[1] == max_raw[d]:
                max_raw_trees.append(parts[0])
            elif parts[1] > max_raw[d]:
                max_raw_trees = [parts[0]]
            if parts[2] == max_opt[d]:
                max_opt_trees.append(parts[0])
                print parts[0]
            elif parts[2] > max_opt[d]:
                print "New opt max:", parts[2]
                max_opt_trees = [parts[0]]
                print parts[0]
            max_raw[d] = max(max_raw[d], int(parts[1]))
            max_opt[d] = max(max_opt[d], int(parts[2]))
            sum_raw[d] = sum_raw[d] + int(parts[1])
            sum_opt[d] = sum_opt[d] + int(parts[2])
        curfile.close()
    if counter > 0:
        sum_raw[d] = float(sum_raw[d]) / float(counter)
        sum_opt[d] = float(sum_opt[d]) / float(counter)
        print "Maximum raw trees"
        for tree in max_raw_trees:
            print tree
        print "Maximum opt trees"
        for tree in max_opt_trees:
            print tree
        print "Trees where both is optimal:"
        for tree in [t for t in max_raw_trees if t in max_opt_trees]:
            print tree

for d in degrees:
    print "Degree %d: mr %d mo %d ar %f ao %f"%(d, max_raw[d], max_opt[d], sum_raw[d], sum_opt[d])


