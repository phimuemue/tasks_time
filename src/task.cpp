#include "task.h"

Task::Task(){
    id = 0;
    elapsed = 0;
    distribution = Exponential;
    exponential_lambda = 1;
}

Task::Task(task_id id, Distribution d, myfloat param1, myfloat param2){
    this->id = id;
    distribution = d;
    elapsed = 0;
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

Task::Task(const Task& t){
    id = t.id;
    elapsed = t.elapsed;
    distribution = t.distribution;
    exponential_lambda = t.exponential_lambda;
    uniform_a = t.uniform_a;
    uniform_b = t.uniform_b;
}

task_id Task::get_id() const {
    return id;
}

void Task::elapse(myfloat t){
    elapsed += t;
}

void Task::set_elapsed(myfloat t){
    elapsed = t;
}

myfloat Task::get_elapsed() const {
    return elapsed;
}

Distribution Task::get_distribution() const{
    return distribution;
}

myfloat Task::get_expected_remaining_time() const {
    switch(distribution){
        case(Exponential):
            // Exponential distribution is memoryless
            return 1/exponential_lambda;
        case(Uniform):
            // Uniform distribution not memoryless
            if(elapsed > uniform_b){        
                throw 0;
            }
            return (uniform_b - elapsed) / 2;
    }
    throw 0;
}

myfloat Task::get_expected_total_time() const {
    return get_expected_remaining_time() + elapsed;
}

bool Task::operator==(const Task& t) const {
    return t.id == id;
}

bool operator==(const task_id& i, const Task& t){
    return t.id == i;
}

bool Task::operator<(const Task& t) const {
    return id < t.id;
}

bool Task::operator>(const Task& t) const {
    return id > t.id;
}

ostream& operator<<(ostream& os, const Task& t){
    os << t.id;
    if(t.elapsed > 0)
        os << "(" << t.elapsed << ")";
    return os;
}
