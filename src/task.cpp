#include "task.h"

Task::Task(int id, Distribution d, myfloat param1, myfloat param2){
    this->id = id;
    distribution = d;
    switch(distribution){
        case(Exponential):
            exponential_lambda = param1;
            break;
        case(Uniform):
            uniform_a = param1;
            uniform_b = param2;
            break;
    }
}

void Task::elapse(myfloat t){
    elapsed += t;
}

void Task::set_elapsed(myfloat t){
    elapsed = t;
}

myfloat Task::get_elapsed(){
    return elapsed;
}

myfloat Task::get_expected_remaining_time(){
    switch(distribution){
        case(Exponential):
            // Exponential distribution is memoryless
            return exponential_lambda;
        case(Uniform):
            // Uniform distribution not memoryless
            if(elapsed > uniform_b){        
                throw 0;
            }
            return (uniform_b - elapsed) / 2;
    }
    throw 0;
}

myfloat Task::get_expected_total_time(){
    return get_expected_remaining_time() + elapsed;
}

bool Task::operator==(const Task& t){
    return t.id == id;
}

ostream& operator<<(ostream& os, const Task& t){
    os << t.id;
    if(t.elapsed > 0)
        os << "(" << t.elapsed << ")";
    return os;
}
