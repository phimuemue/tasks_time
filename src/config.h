#ifndef CONFIG_H
#define CONFIG_H

typedef float myfloat;
typedef int task_id;

enum Distribution {
    Exponential, // needs 1 parameter
    Uniform,     // needs 2 parameter
};

#endif
