#ifndef EXPORTER_H
#define EXPORTER_H

#include "snapshot.h"

#include<ostream>

using namespace std;

class Exporter {
    public:
        virtual void export_snapshot_dag(ostream& output, const Snapshot* s) const = 0;
};

#endif
