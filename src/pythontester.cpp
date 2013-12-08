#include "pythontester.h"

#if PYTHON_TESTS

using namespace boost::python;

PythonTester::PythonTester() :
    module_path("tester")
{
    try {
        Py_Initialize();
        initintree();
        initsnapshot();
        // adjust working directory and search paths -- taken from
        // http://stackoverflow.com/questions/9285384/
        boost::filesystem::path workingDir = boost::filesystem::absolute("./").normalize();
        PyObject* sysPath = PySys_GetObject("path");
        PyList_Insert( sysPath, 0, PyString_FromString(workingDir.string().c_str()));
        boost::python::api::object main_module = import("__main__");

        test_module = import("tester");
    }
    catch (error_already_set const &){
        string err_msg = parse_python_exception();
        cout << err_msg << endl;
    }
}

PythonTester::~PythonTester(){
    Py_Finalize();
}

string PythonTester::test_string(const Snapshot* s){
    string result = "";
    result += test_all(s) ? "Y" : " ";
    result += test_first(s) ? " " : "!";
    return result;
}

bool PythonTester::test(const Snapshot* s){
    bool result = false;
    try{
        //boost::shared_ptr<const Snapshot> ptr_snap(s);
        object testfunc = test_module.attr("test");
        result = testfunc(ptr(s));
    }
    catch (error_already_set){
        string err_msg = parse_python_exception();
        cout << err_msg << endl;
    }
    return result;
}

bool PythonTester::combine_function(const Snapshot* s, bool result, vector<bool> values){
    return true;
}

// this is taken from 
// http://thejosephturner.com/blog/post/embedding-python-in-c-applications-with-boostpython-part-2
std::string PythonTester::parse_python_exception(){
    PyObject *type_ptr = NULL, *value_ptr = NULL, *traceback_ptr = NULL;
    PyErr_Fetch(&type_ptr, &value_ptr, &traceback_ptr);
    std::string ret("Unfetchable Python error");
    if(type_ptr != NULL){
        boost::python::handle<> h_type(type_ptr);
        boost::python::str type_pstr(h_type);
        boost::python::extract<std::string> e_type_pstr(type_pstr);
        if(e_type_pstr.check())
            ret = e_type_pstr();
        else
            ret = "Unknown exception type";
    }
    if(value_ptr != NULL){
        boost::python::handle<> h_val(value_ptr);
        boost::python::str a(h_val);
        boost::python::extract<std::string> returned(a);
        if(returned.check())
            ret +=  ": " + returned();
        else
            ret += std::string(": Unparseable Python error: ");
    }
    if(traceback_ptr != NULL){
        boost::python::handle<> h_tb(traceback_ptr);
        boost::python::object tb(boost::python::import("traceback"));
        boost::python::object fmt_tb(tb.attr("format_tb"));
        boost::python::object tb_list(fmt_tb(h_tb));
        boost::python::object tb_str(boost::python::str("\n").join(tb_list));
        boost::python::extract<std::string> returned(tb_str);
        if(returned.check())
            ret += ": " + returned();
        else
            ret += std::string(": Unparseable Python traceback");
    }
    return ret;
}

#endif
