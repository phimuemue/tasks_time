#include "onlyonenonscheduledsiblingtester.h"

string OnlyOneNonScheduledSiblingTester::test_string(const Snapshot* s){
    string result = "";
    result += test_first(s) ? " " : "U";
    return result;
}

bool OnlyOneNonScheduledSiblingTester::test(const Snapshot* s){
    unsigned int count = 0;
    set<task_id> marked_successors;
    for(task_id it : s->marked){
        marked_successors.insert(s->intree.get_successor(it));
    }
    for(task_id suc : marked_successors){
        vector<task_id> preds;
        s->intree.get_predecessors(suc, preds);
        preds.erase(
                remove_if(preds.begin(), preds.end(),
                    [&](const task_id x) -> bool{
                        vector<task_id> new_preds;
                        s->intree.get_predecessors(x, new_preds);
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
    if(!(count > 1)){
        return !(count > 1);
    }
    bool result = !(count > 1);
    for(auto it : s->Successors){
        result = result && test(it);
    }
    return result;
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

