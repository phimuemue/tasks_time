#ifndef COMMANDLINEEXPORTER_H
#define COMMANDLINEEXPORTER_H

#include<iostream>
#include<iomanip>
#include<deque>
#include "exporter.h"

class CommandLineExporter : public Exporter{
    public:
        virtual void export_snapshot_dag(ostream& output, const Snapshot* s) const;
        void export_tree(ostream& output, const Intree& t) const;
    private:
        void export_helper(
                map<unsigned int, vector<task_id>>& tasks_by_level,
                vector<string>& lines,
                task_id tid,
                unsigned int level,
                const Intree& t,
                const Intree::Outtree* tmp) const;
};

#endif
