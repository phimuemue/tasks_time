#include "info.h"

char builtby[] = "muellerp";
char version[] = "0";
char minor[] = "0";
char build[] = "462";
char date[] = "2013/04/17 11:58:33";

using namespace std;

void print_version(){
    cout << "tasks_time version " << version << "." << minor
         << "." << build
         << " built on " << date
         << " by " << builtby 
         << endl;
}
