#include "info.h"

char builtby[] = "muellerp";
char version[] = "0";
char minor[] = "1";
char build[] = "304";
char date[] = "2013/04/25 11:36:23";

using namespace std;

void print_version(){
    cout << "tasks_time version " << version << "." << minor
         << "." << build
         << " built on " << date
         << " by " << builtby 
         << endl;
}
