#ifndef EXPORTER_H
#define EXPORTER_H

#include<ostream>

#include "snapshot.h"

using namespace std;

class Exporter {
    public:
        virtual void export_snapshot_dag(ostream& output, const Snapshot* s) const = 0;
};

#endif
