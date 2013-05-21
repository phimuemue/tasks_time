#include "dagviewexporter.h"

DagviewExporter::DagviewExporter(){
    task_count_limit = 0;
}

DagviewExporter::DagviewExporter(unsigned int tcl) :
    task_count_limit(tcl)
{
}

DagviewExporter::~DagviewExporter(){

}

void DagviewExporter::export_snapshot_dag(ostream& output, const Snapshot* s) const{
    dag_view_string(s,
            output,
            task_count_limit, 
            0, 
            1);
}

void DagviewExporter::dag_view_string_internal(const Snapshot* s,
        ostream& output,
        unsigned int task_count_limit,
        myfloat probability,
        unsigned int depth) const {
    for(unsigned int i=0; i<depth; ++i){
        output << " ";
    }
    output << *s << " " << s->expected_runtime() << " " << probability << endl;
    if(s->intree.count_tasks() > task_count_limit){
        if(!s->intree.is_chain()){
            auto pit = s->successor_probs.begin();
            for(auto it = s->successors.begin(); it!=s->successors.end(); ++it, ++pit){
                dag_view_string_internal(*it, output, task_count_limit, *pit, depth+1);
            }
        }
    }
}

void DagviewExporter::dag_view_string(const Snapshot* s, 
        ostream& output, 
        unsigned int task_count_limit, 
        unsigned int depth, 
        myfloat probability) const {
    dag_view_string_internal(s, output, task_count_limit, 1, 0);
}

