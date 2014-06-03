#include "singlesuccessortester.h"

string SingleSuccessorTester::test_string(const Snapshot* s){
    string result = "";
    result += test_first(s) ? " " : "P";
    return result;
}

bool SingleSuccessorTester::test(const Snapshot* s){
    if(s->intree.count_tasks()==1){
        return true;
    }
    bool marked_have_siblings = false;
    for(auto it : s->marked){
        if(s->intree.get_siblings(it).size() > 0){
            marked_have_siblings = true;
            break;
        }
    }
    if(marked_have_siblings == false){
        return true;
    }
    vector<task_id> const leaves = s->intree.get_leaves();
    bool has_single_predecessor = false;
    for(auto it : leaves){
        if(s->intree.get_siblings(it).size() == 0){
            has_single_predecessor = true;
            break;
        }
    }
    if(has_single_predecessor == false){
        return true;
    }
    for(auto it : leaves){
        if(find(s->marked.begin(), s->marked.end(), it) == s->marked.end() &&
                s->intree.get_level(it) == s->intree.longest_chain_length() - 1){
            if(s->intree.get_siblings(it).size() == 0){
                return false;
            }
        }
    }
    return true;
}

bool SingleSuccessorTester::combine_function(const Snapshot* s, bool result, vector<bool> values){
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

