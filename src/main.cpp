#include<iostream>

#include "intree.h"

using namespace std;

int main(int argc, char** argv){
    Task t(0, Uniform, 1, 4);
    cout << t << t.get_expected_remaining_time() << 
        " | " << t.get_expected_total_time() << endl;
    t.elapse(1);
    cout << t << t.get_expected_remaining_time() << 
        " | " << t.get_expected_total_time() << endl;
    return 0;
}
