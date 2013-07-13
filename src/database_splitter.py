curcount = -1

database = open("subtree_database_2_20.txt", "r")
curfile = open("subtree_database_-1.txt", "w")
for line in database:
    count = line.count(" ")
    if count > curcount:
        curcount = count
        curfile.close()
        curfile = open("subtree_database_%d.txt"%(count+2), "w")
        print count
    curfile.write(line)
