#ifndef TESTER_H
#define TESTER_H

#include<iostream>
#include<string>
#include<vector>
#include<map>

#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"

#include "snapshot.h"

using namespace std;

template<typename RetVal, typename... Args>
class Tester {
    public:
        virtual RetVal test_first(const Snapshot* s, Args... args);
        virtual RetVal test_all(const Snapshot* s, Args... args);
        virtual string test_string(const Snapshot* s, Args... args) = 0;
    private:
        map<boost::tuple<const Snapshot*, Args...>, RetVal> cache_first;
        map<boost::tuple<const Snapshot*, Args...>, RetVal> cache_all;
        virtual RetVal test(const Snapshot* s, Args... args) = 0;
        virtual RetVal combine_function(
                const Snapshot* s,
                RetVal result,
                vector<RetVal> values) = 0;
};

template<typename RetVal, typename... Args>
RetVal Tester<RetVal, Args...>::test_first(const Snapshot* s, Args... args){
    auto finder = boost::tuple<const Snapshot*, Args...>
        (s, args...);
    if (cache_first.find(finder) != cache_first.end()){
        return cache_first[finder];
    }
    return cache_first[finder] = test(s, args...);
}

template<typename RetVal, typename... Args>
RetVal Tester<RetVal, Args...>::test_all(const Snapshot* s, Args... args){
    auto finder = boost::tuple<const Snapshot*, Args...>
        (s, args...);
    if (cache_all.find(finder) != cache_all.end()){
        return cache_all[finder];
    }
    vector<RetVal> values;
    for(const Snapshot* suc : s->Successors){
        values.push_back(test(suc, args...));
    }
    return cache_all[finder] = combine_function(s, test(s, args...), values);
}

#endif
