#ifndef HLFTESTER_H
#define HLFTESTER_H

#include "tester.h"

class HLFTester : public Tester<HLFTester, bool> {
    public:
        string test_string(const Snapshot* s);
        bool test(const Snapshot* s);
        bool combine_function(const Snapshot* s, bool result, vector<bool> values);
};

#endif
