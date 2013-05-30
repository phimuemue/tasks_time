#ifndef DAGVIEWEXPORTER_H
#define DAGVIEWEXPORTER_H

#include "exporter.h"

class DagviewExporter : public Exporter {
    private:
        void dag_view_string_internal(const Snapshot* s,
                ostream& output,
                unsigned int task_count_limit,
                myfloat probability,
                unsigned int depth) const ;
        void dag_view_string(const Snapshot* s,
                ostream& output,
                unsigned int task_count_limit, 
                unsigned int depth, 
                myfloat probability) const ;
    public:
        unsigned int task_count_limit;
        DagviewExporter();
        DagviewExporter(unsigned int tcl);
        ~DagviewExporter();
        void export_snapshot_dag(ostream& output, const Snapshot* s) const;
};

#endif
