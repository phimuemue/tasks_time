#ifndef SINGLESUCCESSORTESTER_H
#define SINGLESUCCESSORTESTER_H

#include "tester.h"

class SingleSuccessorTester : public Tester<SingleSuccessorTester, bool> {
    public:
        string test_string(const Snapshot* s);
        bool test(const Snapshot* s);
        bool combine_function(const Snapshot* s, bool result, vector<bool> values);
};

#endif
