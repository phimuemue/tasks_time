#ifndef CONFIG_H
#define CONFIG_H

#if RELEASE_MODE
#define NDEBUG
#endif

#include<utility>
#include<vector>
#include<limits>

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

typedef unsigned int task_id;
#if USE_MATULA
typedef unsigned int tree_id;
#else
typedef std::vector<unsigned char> tree_id;
#endif
typedef std::pair<tree_id, std::vector<task_id>> snapshot_id;

#define NOTASK (std::numeric_limits<task_id>::max())

enum Distribution {
    Exponential, // needs 1 parameter
    Uniform,     // needs 2 parameters
};

#endif
