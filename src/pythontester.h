#ifndef PYTHONTESTER_H
#define PYTHONTESTER_H

#if PYTHON_TESTS

#include <string>
#include <boost/python.hpp>
#include <boost/filesystem.hpp>

#include "tester.h"

class PythonTester : public Tester<PythonTester, bool> {
    public:
        PythonTester();
        ~PythonTester();
        string test_string(const Snapshot* s);
        bool test(const Snapshot* s);
        bool combine_function(const Snapshot* s, bool result, vector<bool> values);
    private:
        static void register_types();
        string module_path;
        boost::python::object test_module;

        string parse_python_exception();
};

#endif

#endif
