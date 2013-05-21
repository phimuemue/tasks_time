#include "tikzexporter.h"


TikzExporter::TikzExporter(){
    show_probabilities = true;
    show_expectancy = true;
}

TikzExporter::TikzExporter(bool se, bool sp) :
    show_probabilities(sp), 
    show_expectancy(se)
{
}

TikzExporter::~TikzExporter(){
}

void TikzExporter::export_snapshot(ostream& output, const Snapshot* s) const{

}

