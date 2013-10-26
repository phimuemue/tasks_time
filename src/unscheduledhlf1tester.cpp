#include "unscheduledhlf1tester.h"

string UnscheduledHlf1Tester::test_string(const Snapshot* s){
    string result = "";
    result += test_first(s) ? "2" : " ";
    return result;
}

bool UnscheduledHlf1Tester::test(const Snapshot* s){
    vector<task_id> leaves;
    s->intree.get_leaves(leaves);
    auto depth = s->intree.longest_chain_length() - 1;
    int max_s=0, min_s=s->intree.count_tasks();
    for(auto t : s->marked){
        min_s = min(s->intree.get_level(t), min_s);
        max_s = max(s->intree.get_level(t), max_s);
    }
    for(auto t : leaves){
        if(find(s->marked.begin(), s->marked.end(), t) == s->marked.end()
                && s->intree.get_level(t) > min_s
                && s->intree.get_level(t) == s->intree.longest_chain_length() - 2 
                && s->intree.get_level(t) < max_s
          ){
            return true;
        }
    }
    return false;
}

bool UnscheduledHlf1Tester::combine_function(const Snapshot* s, bool result, vector<bool> values){
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

