#include "info.h"

char builtby[] = "philipp";
char version[] = "0";
char minor[] = "1";
char build[] = "486";
char date[] = "2013/05/02 19:35:21";

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
