#include "info.h"

char builtby[] = "muellerp";
char version[] = "0";
char minor[] = "3";
char build[] = "115";
char date[] = "2013/06/04 15:34:15";

using namespace std;

void print_version(){
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
