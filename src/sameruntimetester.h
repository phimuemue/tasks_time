#ifndef SAMERUNTIMETESTER_H
#define SAMERUNTIMETESTER_H

#include "tester.h"

class SameRunTimeTester : public Tester<SameRunTimeTester, bool> {
    public:
        string test_string(const Snapshot* s);
        bool test(const Snapshot* s);
        bool combine_function(const Snapshot* s, bool result, vector<bool> values);
};


#endif
