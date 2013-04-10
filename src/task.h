#ifndef TASK_H
#define TASK_H

#include "config.h"

/*  A task represents a task to be done, and holds additional information
    so that it can act as a random variable (thus also representing the
    random variable describing the time until the task is finished).
*/
class Task {
private:
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
        // stuff for poisson distribution
        struct {
            myfloat poisson_lambda;
        };
    };
public:
    void elapse(myfloat t);
    void set_elapsed(myfloat t);
    myfloat get_elapsed();
    myfloat get_expected_remaining_time();
};

#endif
