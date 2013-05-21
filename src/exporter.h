#ifndef EXPORTER_H
#define EXPORTER_H

#include<ostream>

#include "snapshot.h"

using namespace std;

class SnapshotExporter {
    public:
        virtual void export_snapshot(ostream& output, const Snapshot* s) const = 0;
};

#endif
