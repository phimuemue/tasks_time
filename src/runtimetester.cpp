#include "runtimetester.h"

string RuntimeTester::test_string(const Snapshot* s, unsigned int p){
    stringstream ss;
    RuntimeTester rtt;
    ss << "(";
    for(unsigned int procs = p; procs>0; --procs){
        ss << test_all(s, procs); 
        if (procs > 1){
            ss << "|";
        }
    }
    ss << ")";
    return ss.str();
}

myfloat RuntimeTester::test(const Snapshot* s, unsigned int p){
    if(p==1 && s->intree.count_tasks() == 1){
        return 1;
    }
    if(s->marked.size() == 0){
        return 1;
    }
    myfloat transition_time = ((myfloat)1)/((myfloat) s->marked.size());
    return (s->marked.size() == p ? transition_time : 0);
}

myfloat RuntimeTester::combine_function(const Snapshot* s, myfloat result, vector<myfloat> values){
    myfloat returnvalue = (myfloat)result;
    auto val = values.begin();
    for(auto sucs = s->Successors().begin();
            sucs != s->Successors().end();
            sucs++, val++)
    {
        myfloat tmp = sucs->probability;
        tmp *= *val;
        returnvalue += tmp;
    }
    return returnvalue;
}

