#include "task.h"

void Task::elapse(myfloat t){
    elapsed += t;
}

void Task::set_elapsed(myfloat t){
    elapsed = t;
}

myfloat Task::get_elapsed(){
    return elapsed;
}

myfloat get_expected_remaining_time(){
    // TODO: to be implemented
    throw 0;
}
