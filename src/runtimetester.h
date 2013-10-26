#ifndef RUNTIMETESTER_H
#define RUNTIMETESTER_H

#include "tester.h"

class RuntimeTester : public Tester<myfloat, unsigned int> {
    public:
        string test_string(const Snapshot* s, unsigned int p);
    private:
        myfloat test(const Snapshot* s, unsigned int p);
        myfloat combine_function(const Snapshot* s, myfloat result, vector<myfloat> values);
};


#endif
