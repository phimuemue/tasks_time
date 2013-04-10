#include "task.h"

Task::Task(int id, Distribution d){
    this->id = id;
    distribution = d;
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

bool Task::operator==(const Task& t){
    return t.id == id;
}

ostream& operator<<(ostream& os, const Task& t){
    os << t.id;
    if(t.elapsed > 0)
        os << "(" << t.elapsed << ")";
    return os;
}
