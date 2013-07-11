#include<vector>
#include<iostream>
#include<map>
#include<string>
#include<algorithm>
#include<sstream>

using namespace std;

class CheapTree {
    public:
        // members
        vector<unsigned int> list;
        map<unsigned int, vector<unsigned int>> precursors;

        // constructor
        CheapTree(unsigned int size) :
            list(size)
        {
            updatePrecursors();
        }
        CheapTree(const vector<unsigned int>& l) :
            list(l)
        {
            updatePrecursors();
        }

        void updatePrecursors(){
            for(unsigned int i = 0; i<list.size()+1; ++i){
                precursors[i] = vector<unsigned int>(0);
            }
            for(unsigned int i = 0; i<list.size(); ++i){
                precursors[list[i]].push_back(i+1);
            }
        }

        // methods
        bool increment(){
            unsigned int size = list.size();
            unsigned int pos = list.size() - 1;
            list[pos]++;
            while (pos < list.size() && list[pos] > pos){
                list[pos] = 0;
                pos--;
                if(pos > list.size()){
                    return false;
                }
                list[pos]++;
                for(unsigned int i=pos+1; i<list.size(); ++i){
                    list[i] = list[pos];
                }
            }
            updatePrecursors();
            return true;
        }

        string get_tree_id() const {
            return get_tree_id(0);
        }

        string get_tree_id(unsigned int t) const
        {
            vector<string> tmp;
            for(auto s : precursors.at(t)){
                tmp.push_back(get_tree_id(s));
            }
            sort(tmp.begin(), tmp.end());
            stringstream ss;
            if(tmp.size() > 0)
            {
                ss << "[";
                for(auto s : tmp){
                    ss << s;
                }
                ss << "]";
            }
            else{
                ss << "o";
            }
            return ss.str();
        }

        unsigned int count_subtrees() const {
            
            return 0;
        }

        unsigned int count_subtrees(map<CheapTree, bool>& done, map<CheapTree, unsigned int> global_pool) const {
            if(done.find(*this) != done.end()){
                return 0;
            }
            if(global_pool.find(*this)!=global_pool.end()){
                return global_pool.at(*this);
            }
            unsigned int result = 0;
            
            return result;
        }

        friend ostream& operator<< (ostream& os, const CheapTree& t){
            os << "[";
            for(auto it = t.list.begin(); it!=t.list.end(); ++it){
                os << *it;
                if(next(it,1) != t.list.end()){
                    os << ", ";
                }
            }
            os << "]";
            return os;
        }
};

inline bool operator< (const CheapTree& l, const CheapTree& r){
    return l.get_tree_id() < r.get_tree_id();
}

int main(int argc, char** argv){
    CheapTree t(atoi(argv[1]));
    map<string, bool> done;
    cout << t << endl;
    while(t.increment()){
        if (done.find(t.get_tree_id()) == done.end()){
            done[t.get_tree_id()] = true;
            cout << t << endl;
            //cout << t.get_tree_id() << endl;
        }
    }
    return 0;
}
