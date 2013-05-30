#ifndef CONFIG_H
#define CONFIG_H

#include<utility>
#include<vector>

#if USE_SIMPLE_OPENMP
#include<omp.h>
#endif

#define FLOAT 0
#define DOUBLE 1
#define RATIONAL_INT 2
#define RATIONAL_LONG 3
#define GNUMP_RATIONAL 4

#if MYFLOAT==RATIONAL_LONG
#include<boost/rational.hpp>
#elif MYFLOAT==RATIONAL_INT
#include<boost/rational.hpp>
#elif MYFLOAT==GNUMP_RATIONAL
#include<gmpxx.h>
#endif

// TODO: Add support for rational<gmplib-bigint>
#if MYFLOAT==FLOAT
typedef float myfloat;
#elif MYFLOAT==DOUBLE
typedef double myfloat;
#elif MYFLOAT==RATIONAL_INT
typedef boost::rational<unsigned int> myfloat;
#elif MYFLOAT==RATIONAL_LONG
typedef boost::rational<unsigned long long> myfloat;
#elif MYFLOAT==GNUMP_RATIONAL
typedef mpq_class myfloat;
#endif 

typedef int task_id;
typedef unsigned long long tree_id;
typedef std::pair<tree_id, std::vector<task_id>> snapshot_id;

#define NOTASK (-1)

enum Distribution {
    Exponential, // needs 1 parameter
    Uniform,     // needs 2 parameters
};

#endif
