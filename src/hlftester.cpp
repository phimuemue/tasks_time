#include "hlftester.h"

string HLFTester::test_string(const Snapshot* s){
    string result = "";
    result += test_all(s) ? "H" : " ";
    result += test_first(s) ? " " : "!";
    return result;
}

bool HLFTester::test(const Snapshot* s){
    if(s->intree.count_tasks()==1){
        return true;
    }
    vector<task_id> leaves;
    s->intree.get_leaves(leaves);
    vector<unsigned int> leaf_levels;
    for(auto t : leaves){
        leaf_levels.push_back(s->intree.get_level(t));
    }
    sort(leaf_levels.begin(), leaf_levels.end(), greater<task_id>());
    vector<unsigned int> m_levels;
    for(auto t : s->marked){
        m_levels.push_back(s->intree.get_level(t));
    }
    sort(m_levels.begin(), m_levels.end(), greater<task_id>());
    for(unsigned int i=0; i<m_levels.size(); ++i){
        if(m_levels[i]<leaf_levels[i]){
            return false;
        }
    }
    return true;
}

bool HLFTester::combine_function(const Snapshot* s, bool result, vector<bool> values){
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

