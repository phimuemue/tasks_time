#ifndef TIKZ_TO_PDF
#define TIKZ_TO_PDF

#include<string>
#include<iostream>
#include<stdio.h>

class TikzToPdf {
    public:
        TikzToPdf();
        TikzToPdf(const TikzToPdf& t);
        ~TikzToPdf();
        void convertTikzToPdf(std::string infile, std::string outfile);
};

#endif
