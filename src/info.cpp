#include "info.h"

char builtby[] = "philipp";
char version[] = "0";
char minor[] = "4";
char build[] = "734";
char date[] = "2013/09/15 10:17:15";

using namespace std;

void print_version(bool long_version){
    if(long_version){
        cout << "-----------------------------------------------------------------" << endl;
        cout << "+  " << endl << 
            "+         o---o---o---o---o-\\           " << endl <<
            "+                            \\          tasks_time" << endl <<
            "+   o---o---o-\\     o-\\       \\         " << version << "." << minor << "." << build << endl <<
            "+              \\       \\       \\     " << endl <<
            "+   o---o---o---o---o---o---o---o---o   " << date << endl <<
            "+              /           /            " << builtby << endl <<
            "+   o---o---o-/     o---o-/             " << endl << 
            "+  " << endl;
        cout << "-----------------------------------------------------------------" << endl;
        cout << "+   tasks_time " << version << "." << minor
            << "." << build
            << " built " << date
            << " by " << builtby 
            << endl;
        cout << "-----------------------------------------------------------------" << endl << endl;
    }
    else {
        cout << "tasks_time " << version << "." << minor
            << "." << build
            << "(" << date
            << ", " << builtby << ")"
            << endl;
    }
}
