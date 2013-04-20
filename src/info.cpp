#include "info.h"

char builtby[] = "philipp";
char version[] = "0";
char minor[] = "0";
char build[] = "678";
char date[] = "2013/04/20 14:56:33";

using namespace std;

void print_version(){
    cout << "tasks_time version " << version << "." << minor
         << "." << build
         << " built on " << date
         << " by " << builtby 
         << endl;
}
