#include "info.h"

char builtby[] = "philipp";
char version[] = "0";
char minor[] = "0";
char build[] = "683";
char date[] = "2013/04/20 18:10:48";

using namespace std;

void print_version(){
    cout << "tasks_time version " << version << "." << minor
         << "." << build
         << " built on " << date
         << " by " << builtby 
         << endl;
}
