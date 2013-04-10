#ifndef TASK_H
#define TASK_H

#include<iostream>

#include "config.h"

using namespace std;

/*  A task represents a task to be done, and holds additional information
    so that it can act as a random variable (thus also representing the
    random variable describing the time until the task is finished).
*/
class Task {
private:
    task_id id;
    myfloat elapsed;
    Distribution distribution;
    union {
        // stuff for exponential distribution
        struct {
            myfloat exponential_lambda;
        };
        // stuff for uniform distribution
        struct {
            myfloat uniform_a;
            myfloat uniform_b;
        };
    };
public:
    Task();
    Task(task_id id, Distribution d = Exponential, myfloat param1 = 1, myfloat param2 = 2);
    task_id get_id() const;

    void elapse(myfloat t);
    void set_elapsed(myfloat t);
    myfloat get_elapsed();

    myfloat get_expected_remaining_time();
    myfloat get_expected_total_time();

    bool operator==(const Task& t) const;
    bool operator>(const Task& t) const;
    bool operator<(const Task& t) const;
    friend bool operator==(const int& i, const Task& t);

    friend ostream& operator<<(ostream& os, const Task& t);
};

#endif
