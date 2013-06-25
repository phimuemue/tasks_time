#ifndef PROFILEEXPORTER_H
#define PROFILEEXPORTER_H

#include "newtikzexporter.h"

class ProfileExporter : public TikzExporter2 {

    protected:
        virtual void export_single_snapshot_internal(ostream& output,
                const Snapshot& s,
                const task_id t,
                const map<task_id, vector<task_id>>& rt,
                const unsigned int depth,
                const float leftoffset) const;
        bool same_profile(const TikzNode* a, const TikzNode *b) const;
        virtual void consolidate_levels(map<unsigned int, vector<const TikzNode*>>& levels) const ;
};

#endif
