#include "info.h"

char builtby[] = "muellerp";
char version[] = "0";
char minor[] = "0";
char build[] = "559";
char date[] = "2013/04/18 12:53:36";

using namespace std;

void print_version(){
    cout << "tasks_time version " << version << "." << minor
         << "." << build
         << " built on " << date
         << " by " << builtby 
         << endl;
}
