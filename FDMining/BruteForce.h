#ifndef BruteForce_h
#define BruteForce_h

#include <vector>
#include <string>
#include <unordered_map>

struct pairhash
{
public:
    template <typename T, typename U>
    std::size_t operator()(const std::pair<T, U> &x) const
    {
        return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
    }
};

class BruteForce
{
private:
    const static int ROW_COUNT = 100;
    const static int COLUMN_COUNT = 12;
    const static int PER_LAYER_COUNT = 6300;
    
private:
    std::unordered_map<std::string, int>* mapToHash;
    int** hashedData;
    int* calculatedPartition;
    bool** prevLayerUsed;
    int*** prevLayerPartition;
    int* location;
    bool* used;
    
public:
    static std::vector<std::string> split(const std::string& st)
    {
        std::vector<std::string> v;
        
        int l = (int)st.size();
        int prev = 0;
        for (int i = 0; i < l; ++i)
        {
            if (st[i] == ',' && st[i + 1] != ' ')
            {
                // split!
                v.push_back(st.substr(prev, i - prev));
                prev = i + 1;
            }
        }
        
        return v;
    }
    
    static bool cmp(const int& a, const int& b)
    {
        return __builtin_popcount(a) < __builtin_popcount(b) || (__builtin_popcount(a) == __builtin_popcount(b) && a < b);
    }
    
    static void output(int lhs, int rhs)
    {
        for (int i = 0; i < COLUMN_COUNT; ++i)
        {
            if (lhs & (1 << i))
            {
                std::cout << i + 1 << " ";
            }
        }
        std::cout << "-> " << rhs + 1 << "\n";
    }
    
    static int lowbit(int x)
    {
        return x & (-x);
    }
    
private:
    void giveMeFive(int rhs);
    int calcPI(int layer, int dj);
    int mergePI(int layer1, int dj1, int layer2, int dj2, int dj3);
    int getPI(int layer1, int dj1, int tick, int location);
    void updatePI(int pi, int location);
    void floodfill(int binrep);
    
public:
    BruteForce(const std::string& fileName);
    ~BruteForce();
    
    void execute();
};

#endif
