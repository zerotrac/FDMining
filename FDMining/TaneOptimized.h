#ifndef TaneOptimized_h
#define TaneOptimized_h

#include <vector>
#include <string>

#define _bp __builtin_popcount

class TaneOptimized
{
private:
    const static int ROW_COUNT_SMALL = 100;
    const static int COLUMN_COUNT_SMALL = 12;
    
    const static int ROW_COUNT_LARGE = 99918;
    const static int COLUMN_COUNT_LARGE = 15;
    
private:
    std::vector<int> subsets;
    std::vector<int> part;
    char buf[1 << 20];
    int presubl;
    
private:
    std::vector<std::vector<int>>* partition;
    int* rhsPlus;
    int* partitionSize;
    
    std::vector<int>* S;
    int* T;
    
    std::string inputFileName;
    std::string outputFileName;
    
    // store the found FDs without sorting
    std::vector<std::vector<int>> ans;
    
    // store the results of dividing by set length
    std::vector<int> binRepresent;
    bool* available;
    int* binL;
    int* binR;
    
    int exactRowCount, exactColumnCount;
    
public:
    TaneOptimized(const std::string& _inputFileName, const std::string& _outputFileName, int smallOrLarge);
    ~TaneOptimized();
    
private:
    static bool cmpBySize(const int& a, const int& b)
    {
        return _bp(a) < _bp(b) || (_bp(a) == _bp(b) && a < b);
    }
    
    static bool cmpByLexico(const std::vector<int>& a, const std::vector<int>& b)
    {
        int la = (int)a.size();
        int lb = (int)b.size();
        int l = std::min(la, lb);
        
        for (int i = 0; i < l; ++i)
        {
            if (a[i] != b[i])
            {
                return a[i] < b[i];
            }
        }
        return la < lb;
    }
    
    static void split(std::vector<std::string>& v, const std::string& st)
    {
        v.clear();
        size_t l = st.size(), prev = 0;
        for (int i = 0; i < l; ++i)
        {
            if (st[i] == ',' && st[i + 1] != ' ')
            {
                // split!
                v.push_back(st.substr(prev, i - prev));
                prev = i + 1;
            }
        }
    }
    
    static int lowbit(int x)
    {
        return x & (-x);
    }
    
private:
    void prepareLayers();
    void prepareData();
    void getSubsets(std::vector<int>& subsets, int bin);
    bool checkAllSubsets(const std::vector<int>& subsets);
    bool strippedProduct(int bin, int subl, int subr, int presubl);
    void computeDependencies(const std::vector<int>& subsets, int bin);
    void output(int lhs, int rhs);
    
public:
    void execute();
};

#endif
