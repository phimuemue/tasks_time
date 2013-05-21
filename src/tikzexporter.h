#ifndef TIKZEXPORTER_H
#define TIKZEXPORTER_H

#include "exporter.h"

class TikzExporter : public SnapshotExporter {
    private:
        bool show_probabilities;
        bool show_expectancy;
    public:
        TikzExporter();
        TikzExporter(bool se, bool sp);
        ~TikzExporter();
        void export_snapshot(ostream& output, const Snapshot* s) const;
};

#endif
