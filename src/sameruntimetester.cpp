#include "sameruntimetester.h"

string SameRunTimeTester::test_string(const Snapshot* s){
    string result = "";
    result += test_first(s) ? "S" : " ";
    return result;
}

bool SameRunTimeTester::test(const Snapshot* s){
    myfloat epsilon = (myfloat)0.00001;
    for(auto suc1 = s->Successors.begin(); suc1 != s->Successors.end(); ++suc1) {
        for(auto suc2 = suc1 + 1; suc2 != s->Successors.end(); ++suc2) {
            if(abs((*suc1)->expected_runtime() - (*suc2)->expected_runtime()) < epsilon)
            {
                vector<const Snapshot*> s1;
                vector<const Snapshot*> s2;
                for(auto it : (*suc1)->Successors){
                    s1.push_back(it);
                }
                for(auto it : (*suc2)->Successors){
                    s2.push_back(it);
                }
                sort(s1.begin(), s1.end());
                sort(s2.begin(), s2.end());
                if(s1 != s2){
                    return true;
                }
            }
        }
    }
    return false;
}

bool SameRunTimeTester::combine_function(const Snapshot* s, bool result, vector<bool> values){
    if(!result){
        return true;
    }
    for(auto it : values){
        if(it){
            return true;
        }
    }
    return false;
}

