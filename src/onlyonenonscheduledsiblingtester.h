#ifndef ONLYONENONSCHEDULEDSIBLING_H
#define ONLYONENONSCHEDULEDSIBLING_H

#include "tester.h"

class OnlyOneNonScheduledSiblingTester : public Tester<OnlyOneNonScheduledSiblingTester, bool> {
    public:
        string test_string(const Snapshot* s);
        bool test(const Snapshot* s);
        bool combine_function(const Snapshot* s, bool result, vector<bool> values);
};

#endif
