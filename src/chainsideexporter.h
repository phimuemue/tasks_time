#ifndef CHAINSIDEEXPORTER_H
#define CHAINSIDEEXPORTER_H

#include "newtikzexporter.h"

class ChainSideExporter : public TikzExporter2 {

    protected:
        virtual void export_single_snapshot_internal(ostream& output,
                const Snapshot& s,
                const task_id t,
                const map<task_id, vector<task_id>>& rt,
                const unsigned int depth,
                const float leftoffset) const;

};

#endif
