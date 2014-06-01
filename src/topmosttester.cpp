#include "topmosttester.h"

string TopmostTester::test_string(const Snapshot* s){
    string result = "";
    result += test_all(s) ? "  " : "TM";
    result += test_first(s) ? " " : "!";
    return result;
}

bool TopmostTester::test(const Snapshot* s){
    vector<task_id> const leaves = s->intree.get_leaves();
    unsigned int max_level = s->intree.longest_chain_length() - 1;
    vector<task_id> topmosttasks;
    for(auto it : leaves){
        if(s->intree.get_level(it) == max_level){
            topmosttasks.push_back(it);
        }
    }
    // if less topmost than processors -> all topmost shall be scheduled
    if(topmosttasks.size() <= s->marked.size()){
        for(auto it : topmosttasks){
            if(find(s->marked.begin(), s->marked.end(), it) == s->marked.end()){
                return false;
            }
        }
    }
    // if more topmost than processors -> all scheduled must be topmost
    else{
        for(auto it : s->marked){
            if(s->intree.get_level(it) != max_level){
                return false;
            }
        }
    }
    return true;
}

bool TopmostTester::combine_function(const Snapshot* s, bool result, vector<bool> values){
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

