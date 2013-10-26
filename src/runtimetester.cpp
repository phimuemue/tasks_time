#include "runtimetester.h"

string RuntimeTester::test_string(const Snapshot* s, unsigned int p){
    string result = "";
    result += to_string(test_first(s, p));
    return result;
}

myfloat RuntimeTester::test(const Snapshot* s, unsigned int p){
    if(p==1 && s->intree.count_tasks() == 1){
        return 1;
    }
    myfloat result = (myfloat)0;
    for(auto it : s->SuccessorProbabilities){
        myfloat tmp = it.get<1>();
        myfloat transition_time = ((myfloat)1)/((myfloat) s->marked.size());
        myfloat future_time = test(it.get<0>(), p);
        tmp *= ((s->marked.size() == p ? transition_time : 0) + future_time);
        result += tmp;
    }
    return result;
}

myfloat RuntimeTester::combine_function(const Snapshot* s, myfloat result, vector<myfloat> values){
    return 0;
}

