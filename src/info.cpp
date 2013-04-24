#include "info.h"

char builtby[] = "philipp";
char version[] = "0";
char minor[] = "1";
char build[] = "250";
char date[] = "2013/04/24 20:30:11";

using namespace std;

void print_version(){
    cout << "tasks_time version " << version << "." << minor
         << "." << build
         << " built on " << date
         << " by " << builtby 
         << endl;
}
