#ifndef TOPMOSTTESTER_H
#define TOPMOSTTESTER_H

#include "tester.h"

class TopmostTester : public Tester<TopmostTester, bool> {
    public:
        string test_string(const Snapshot* s);
        bool test(const Snapshot* s);
        bool combine_function(const Snapshot* s, bool result, vector<bool> values);
};

#endif
