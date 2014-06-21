#include "simpleexporter.h"

SimpleExporter::SimpleExporter(){

}

SimpleExporter::~SimpleExporter(){

}

void SimpleExporter::export_snapshot_dag(ostream& output, const Snapshot* s) const{
    print_snapshot_dag(s, 0, output);
}

void SimpleExporter::print_snapshot_dag(const Snapshot* s, int depth, ostream& output) const {
    for(int i=-1; i<depth; ++i){
        output << "*";
    } 
    output << " ";
    output << *s;
    output << endl;
    for(auto const& it : s->Successors()){
        print_snapshot_dag(it.snapshot, depth+1, output);
    }
}

