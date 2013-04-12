#ifndef CONFIG_H
#define CONFIG_H

typedef float myfloat;
typedef int task_id;

#define NOTASK (-1)

enum Distribution {
    Exponential, // needs 1 parameter
    Uniform,     // needs 2 parameter
};

#endif
