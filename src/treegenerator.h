#ifndef TREEGENERATOR_H
#define TREEGENERATOR_H

#include<python2.7/Python.h>
#include<vector>
#include<string>

class TreeGenerator {
    public:
        static std::vector<std::string> generate_trees(
                unsigned int n, 
                unsigned int minidx=0,
                unsigned int maxidx=0
                );
};

#endif
