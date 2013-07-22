#include "tikztopdf.h"

using namespace std;

TikzToPdf::TikzToPdf(){
}

TikzToPdf::TikzToPdf(const TikzToPdf& t){
}

TikzToPdf::~TikzToPdf(){
}

void TikzToPdf::convertTikzToPdf(string infile, string outfile){
    string tex_code = 
        R"(\documentclass[preview]{standalone})"
        R"(\newcounter{chapter})"
        R"(\input{thesis/packages})"
        R"(\input{thesis/commands_and_environments})"
        R"(\begin{document})"
        R"(\input{default.tex})"
        R"(\end{document})"
        ;
    string command = "pdflatex -interaction=batchmode -jobname=default \"" + tex_code + "\"";
    cout << command << endl;
    FILE* pdflatex = popen(command.c_str(), "r");
    pclose(pdflatex);
}
