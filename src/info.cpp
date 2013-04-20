#include "info.h"

char builtby[] = "philipp";
char version[] = "0";
char minor[] = "0";
char build[] = "687";
char date[] = "2013/04/21 00:27:19";

using namespace std;

void print_version(){
    cout << "tasks_time version " << version << "." << minor
         << "." << build
         << " built on " << date
         << " by " << builtby 
         << endl;
}
