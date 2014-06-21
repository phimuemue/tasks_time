#pragma once

#include<string>

namespace stringhelper {
    template<typename Seq, typename Delim, typename Conv>
    string join(Seq seq, Delim delim, Conv convert) {
        ostringstream oss;
        for(auto it = std::begin(seq); it!=std::end(seq); ++it) {
            oss << convert(*it);
            if (std::next(it)!=std::end(seq)) {
                oss << delim;
            }
        }
        return oss.str();
    }

    template<typename Seq, typename Delim>
    string join(Seq seq, Delim delim) {
        typedef typename Seq::value_type val;
        return join(seq, delim, [](val v) {return v;});
    }
}
