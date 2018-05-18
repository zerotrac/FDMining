#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <cmath>
#include <queue>

#include "BruteForce.h"

BruteForce::BruteForce(const std::string& fileName)
{
    hashedData = new int*[COLUMN_COUNT];
    for (int i = 0; i < COLUMN_COUNT; ++i)
    {
        hashedData[i] = new int[ROW_COUNT];
    }
    mapToHash = new std::unordered_map<std::string, int>[COLUMN_COUNT];

    calculatedPartition = new int[1 << COLUMN_COUNT];
    std::memset(calculatedPartition, 0, sizeof(int) * (1 << COLUMN_COUNT));

    prevLayerUsed = new bool*[2];
    for (int i = 0; i < 2; ++i)
    {
        prevLayerUsed[i] = new bool[PER_LAYER_COUNT];
    }
    
    prevLayerPartition = new int**[2];
    for (int i = 0; i < 2; ++i)
    {
        prevLayerPartition[i] = new int*[PER_LAYER_COUNT];
        for (int j = 0; j < PER_LAYER_COUNT; ++j)
        {
            prevLayerPartition[i][j] = new int[ROW_COUNT];
        }
    }
    
    location = new int[1 << COLUMN_COUNT];
    
    used = new bool[1 << COLUMN_COUNT];
    
    std::ifstream in(fileName);
    std::string st;
    
    int curRow = 0;
    while (std::getline(in, st))
    {
        st = st + ',';
        std::vector<std::string> v = split(st);
        
        for (int column = 0; column < v.size(); ++column)
        {
            std::string cur = v[column];
            if (mapToHash[column].find(cur) == mapToHash[column].end())
            {
                hashedData[column][curRow] = (int)mapToHash[column].size();
                mapToHash[column][cur] = (int)mapToHash[column].size();
            }
            else
            {
                hashedData[column][curRow] = mapToHash[column][cur];
            }
        }
        ++curRow;
    }
    in.close();
}

BruteForce::~BruteForce()
{
    for (int i = 0; i < COLUMN_COUNT; ++i)
    {
        mapToHash[i].clear();
    }
    delete[] mapToHash;
    
    for (int i = 0; i < ROW_COUNT; ++i)
    {
        delete[] hashedData[i];
    }
    delete[] hashedData;
    
    delete[] calculatedPartition;
    
    for (int i = 0; i < 2; ++i)
    {
        delete[] prevLayerUsed[i];
    }
    delete[] prevLayerUsed;
    
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < PER_LAYER_COUNT; ++j)
        {
            delete[] prevLayerPartition[i][j];
        }
        delete[] prevLayerPartition[i];
    }
    delete[] prevLayerPartition;
    
    delete[] location;
    
    delete[] used;
}

void BruteForce::giveMeFive(int rhs)
{
    std::memset(used, false, sizeof(bool) * (1 << COLUMN_COUNT));
    std::memset(location, 0, sizeof(int) * (1 << COLUMN_COUNT));
    
    std::vector<int> binRepresent;
    for (int i = 0; i < (1 << COLUMN_COUNT); ++i)
    {
        if (!(i & (1 << rhs)))
        {
            binRepresent.push_back(i);
        }
    }
    
    std::sort(binRepresent.begin(), binRepresent.end(), cmp);
    
    location[0] = -1;
    for (int i = 0; i < binRepresent.size(); ++i)
    {
        location[binRepresent[i]] = i;
    }
    
    int vl[COLUMN_COUNT], vr[COLUMN_COUNT];
    vl[1] = 0;
    for (int i = 1; i < binRepresent.size(); ++i)
    {
        int x = __builtin_popcount(binRepresent[i - 1]);
        int y = __builtin_popcount(binRepresent[i]);
        if (x != y)
        {
            vl[y] = i;
        }
        vr[y] = i;
    }

    for (int i = 1; i < COLUMN_COUNT; ++i)
    {
        std::cout << "column = " << i << "\n";
        int positive = 0;
        int layer = i & 1;
        
        for (int j = vl[i]; j <= vr[i]; ++j)
        {
            // std::cout << "j = " << j << " " << vl[i] << " " << vr[i] << "\n";
            int dj = j - vl[i];
            if (i == 1)
            {
                int tick = 0, tickt = 1;
                while (tickt != binRepresent[j])
                {
                    ++tick;
                    tickt <<= 1;
                }
                for (int k = 0; k < ROW_COUNT; ++k)
                {
                    prevLayerPartition[layer][dj][k] = hashedData[tick][k];
                }
                
                int leftPI = calcPI(layer, dj);
                int rightPI = getPI(layer, dj, rhs, binRepresent[j] | (1 << rhs), leftPI);
                
                // std::cout << "j = " << j << " " << binRepresent[j] << " " << leftPI << " " << rightPI << "\n";
                if (leftPI == rightPI)
                {
                    output(binRepresent[j], rhs);
                    prevLayerUsed[layer][dj] = false;
                }
                else
                {
                    ++positive;
                    prevLayerUsed[layer][dj] = true;
                }
                
                updatePI(leftPI, binRepresent[j]);
                if (leftPI == ROW_COUNT)
                {
                    floodfill(leftPI);
                }
                if (rightPI == ROW_COUNT)
                {
                    floodfill(rightPI);
                }
            }
            else
            {
                bool judge = true;
                for (int k = 0; k < COLUMN_COUNT; ++k)
                {
                    if (binRepresent[j] & (1 << k))
                    {
                        int tick = binRepresent[j] ^ (1 << k);
                        if (!prevLayerUsed[1 - layer][location[tick] - vl[i - 1]])
                        {
                            // std::cout << "judge false\n";
                            judge = false;
                            break;
                        }
                    }
                }
                
                if (!judge)
                {
                    prevLayerUsed[layer][dj] = false;
                    continue;
                }
                
                std::cout << "j = " << j << " " << vl[i] << " " << vr[i] << "\n";
                
                int originx = binRepresent[j] - lowbit(binRepresent[j]);
                int originy = lowbit(binRepresent[j]) + originx - lowbit(originx);
                
                int ox = location[originx] - vl[i - 1];
                int oy = location[originy] - vl[i - 1];
                // clock_t ta = clock();
                int leftPI = mergePI(layer, dj, 1 - layer, ox, oy);
                // clock_t tb = clock();
                // std::cout << "leftPI = " << leftPI << "\n";
                int rightPI = getPI(layer, dj, rhs, binRepresent[j] | (1 << rhs), leftPI);
                // clock_t tc = clock();
                // double da = (double)(tb - ta) / CLOCKS_PER_SEC;
                // double db = (double)(tc - tb) / CLOCKS_PER_SEC;
                // std::cout << "dut = " << da << " " << db << "\n";
                // std::cout << "rightPI = " << leftPI << "\n";
                // if (leftPI == rightPI) std::cout << "l, r = " << leftPI << " " << rightPI << "\n";
                if (leftPI == rightPI)
                {
                    // std::cout << "location = " << binRepresent[j] << " " << dj << "\n";
                    output(binRepresent[j], rhs);
                    prevLayerUsed[layer][dj] = false;
                }
                else
                {
                    ++positive;
                    prevLayerUsed[layer][dj] = true;
                }
                
                updatePI(leftPI, binRepresent[j]);
                if (leftPI == ROW_COUNT)
                {
                    floodfill(leftPI);
                }
                if (rightPI == ROW_COUNT)
                {
                    floodfill(rightPI);
                }
            }
        }
        
        if (!positive)
        {
            break;
        }
    }
}

int BruteForce::calcPI(int layer, int dj)
{
    int mx = -1;
    for (int i = 0; i < ROW_COUNT; ++i)
    {
        mx = std::max(mx, prevLayerPartition[layer][dj][i]);
    }
    return mx + 1;
}

int BruteForce::mergePI(int layer1, int dj1, int layer2, int dj2, int dj3)
{
    std::unordered_map<std::pair<int, int>, int, pairhash> s;
    for (int i = 0; i < ROW_COUNT; ++i)
    {
        // if (i % 10000 == 0) std::cout << "mergepi i = " << i << "\n";
        std::pair<int, int> pii = std::make_pair(prevLayerPartition[layer2][dj2][i], prevLayerPartition[layer2][dj3][i]);
        if (s.find(pii) == s.end())
        {
            prevLayerPartition[layer1][dj1][i] = (int)s.size();
            s[pii] = (int)s.size();
        }
        else
        {
            prevLayerPartition[layer1][dj1][i] = s[pii];
        }
    }
    return (int)s.size();
}

int BruteForce::getPI(int layer1, int dj1, int tick, int location, int left)
{
    // std::cout << "location = " << layer1 << " " << dj1 << " " << tick << " " << location << " " << calculatedPartition[location] << "\n";
    if (calculatedPartition[location] > 0)
    {
        return calculatedPartition[location];
    }
    
    std::unordered_map<std::pair<int, int>, int, pairhash> s;
    for (int i = 0; i < ROW_COUNT; ++i)
    {
        // if (i % 10000 == 0) std::cout << "getpi i = " << i << "\n";
        std::pair<int, int> pii = std::make_pair(prevLayerPartition[layer1][dj1][i], hashedData[tick][i]);
        //std::cout << "pair = " << prevLayerPartition[layer1][dj1][i] << " " << hashedData[tick][i] << "\n";
        if (s.find(pii) == s.end())
        {
            s[pii] = (int)s.size();
            if (s.size() > left) return -1;
        }
        
    }
    return calculatedPartition[location] = (int)s.size();
}

void BruteForce::updatePI(int pi, int location)
{
    calculatedPartition[location] = pi;
}

void BruteForce::floodfill(int binrep)
{
    std::queue<int> q;
    q.push(binrep);
    while (!q.empty())
    {
        int cur = q.front();
        q.pop();
        
        for (int i = 0; i < COLUMN_COUNT; ++i)
        {
            if (!(cur & (1 << i)) && (!calculatedPartition[cur | (1 << i)]))
            {
                calculatedPartition[cur | (1 << i)] = ROW_COUNT;
                q.push(cur | (1 << i));
            }
        }
    }
}

void BruteForce::output(int lhs, int rhs)
{
    /*for (int i = 0; i < COLUMN_COUNT; ++i)
     {
     if (lhs & (1 << i))
     {
     std::cout << i + 1 << " ";
     }
     }
     std::cout << "-> " << rhs + 1 << "\n";*/
    std::vector<int> part;
    for (int i = 0; i < COLUMN_COUNT; ++i)
    {
        if (lhs & (1 << i))
        {
            part.push_back(i + 1);
        }
    }
    part.push_back(rhs + 1);
    ans.push_back(part);
}

void BruteForce::execute()
{
    for (int i = 0; i < COLUMN_COUNT; ++i)
    {
        std::cout << "i = " << i << "\n";
        giveMeFive(i);
    }
    
    // print
    
    sort(ans.begin(), ans.end(), cmp2);
    std::cout << "count = " << ans.size() << "\n";
    std::ofstream out("testdata/data_output.txt");
    for (const std::vector<int>& part: ans)
    {
        for (int i = 0; i < part.size() - 1; ++i)
        {
            out << part[i] << " ";
        }
        out << "-> " << part[part.size() - 1] << "\n";
    }
    out.close();
}
