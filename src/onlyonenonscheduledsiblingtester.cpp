#include "onlyonenonscheduledsiblingtester.h"

string OnlyOneNonScheduledSiblingTester::test_string(const Snapshot* s){
    string result = "";
    result += test_first(s) ? " " : "U";
    return result;
}

bool OnlyOneNonScheduledSiblingTester::test(const Snapshot* s){
    unsigned int count = 0;
    for(auto it : s->marked){
        if(s->intree.get_level(it) != s->intree.get_level(s->marked[0])){
            return true;
        }
    }
    set<task_id> marked_successors;
    for(task_id it : s->marked){
        marked_successors.insert(s->intree.get_successor(it));
    }
    bool threechildren = false;
    for(auto it : marked_successors){
        if(s->intree.get_predecessors(it).size() >= 3){
            threechildren = true;
        }
    }
    if(threechildren == false){
        return true;
    }
    for(task_id suc : marked_successors){
        vector<task_id> preds = s->intree.get_predecessors(suc);
        preds.erase(
                remove_if(preds.begin(), preds.end(),
                    [&](const task_id x) -> bool{
                        vector<task_id> new_preds = s->intree.get_predecessors(x);
                        return new_preds.size() > 0;
                    }
                    ), 
                preds.end()
                );
        bool a = find(s->marked.begin(), s->marked.end(), preds[0])!=s->marked.end();
        for(task_id pred : preds){
            bool b = find(s->marked.begin(), s->marked.end(), pred)!=s->marked.end();
            if(b != a){
                count = count + 1;
                break;
            }
            a = b;
        }
    }
    return count<=1;
}

bool OnlyOneNonScheduledSiblingTester::combine_function(const Snapshot* s, bool result, vector<bool> values){
    if(!result){
        return false;
    }
    for(auto it : values){
        if(!it){
            return false;
        }
    }
    return true;
}

