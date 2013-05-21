#ifndef SIMPLEEXPORTER_H
#define SIMPLEEXPORTER_H

#include "exporter.h"

class SimpleExporter : public Exporter {
    private:
        void print_snapshot_dag(const Snapshot* s, 
                int depth, 
                ostream& output) const;
    public:
        SimpleExporter();
        ~SimpleExporter();
        void export_snapshot_dag(ostream& output, const Snapshot* s) const;
};


#endif
