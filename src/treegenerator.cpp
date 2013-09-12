#include"treegenerator.h"

#include<iostream>
#include<sstream>

using namespace std;

vector<string> TreeGenerator::generate_trees(unsigned int n, unsigned int minidx, unsigned int maxidx){
    vector<string> result;
    if(n == 1){
        result.push_back("");
    }
    else {
        for(unsigned int i = minidx; i < maxidx+1; ++i){
            for(auto it : TreeGenerator::generate_trees(n-1, i, maxidx+1)){
                ostringstream num;
                num << i;
                result.push_back(num.str() + " " + it);
            }
        }
    }
    return result;
}
