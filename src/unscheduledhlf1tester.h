#ifndef USNCHEDULEDHLFONETESTER_H
#define USNCHEDULEDHLFONETESTER_H

#include "tester.h"

class UnscheduledHlf1Tester : public Tester<UnscheduledHlf1Tester, bool> {
    public:
        string test_string(const Snapshot* s);
        bool test(const Snapshot* s);
        bool combine_function(const Snapshot* s, bool result, vector<bool> values);
};


#endif
