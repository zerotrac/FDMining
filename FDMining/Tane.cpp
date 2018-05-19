#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "Tane.h"
// #include "Debug.h"


Tane::Tane(const std::string& _inputFileName, const std::string& _outputFileName, int smallOrLarge)
{
    inputFileName = _inputFileName;
    outputFileName = _outputFileName;
    
    if (smallOrLarge) // true = large
    {
        exactRowCount = ROW_COUNT_LARGE;
        exactColumnCount = COLUMN_COUNT_LARGE;
    }
    else
    {
        exactRowCount = ROW_COUNT_SMALL;
        exactColumnCount = COLUMN_COUNT_SMALL;
    }
    
    prepareLayers();
    prepareData();
}

Tane::~Tane()
{
    for (int i = 0; i < (1 << exactColumnCount); ++i)
    {
        partition[i].clear();
    }
    delete[] partition;
    delete[] rhsPlus;
    delete[] partitionSize;
    
    delete[] available;
    delete[] binL;
    delete[] binR;
}


void Tane::prepareLayers()
{
    // divide sets by their length |S|
    
    binRepresent.clear();
    for (int i = 0; i < (1 << exactColumnCount); ++i)
    {
        binRepresent.push_back(i);
    }
    std::sort(binRepresent.begin(), binRepresent.end(), cmpBySize);
    
    available = new bool[1 << exactColumnCount];
    std::memset(available, true, sizeof(bool) * (1 << exactColumnCount));
    
    binL = new int[exactColumnCount + 1];
    binR = new int[exactColumnCount + 1];
    binL[0] = binR[0] = 0;
    
    for (int i = 1; i < binRepresent.size(); ++i)
    {
        int x = _bp(binRepresent[i - 1]);
        int y = _bp(binRepresent[i]);
        if (x != y)
        {
            binL[y] = i;
        }
        binR[y] = i;
    }
}

void Tane::prepareData()
{
    partition = new std::vector<std::vector<int>>[1 << exactColumnCount];
    rhsPlus = new int[1 << exactColumnCount];
    for (int i = 0; i < (1 << exactColumnCount); ++i)
    {
        rhsPlus[i] = (1 << exactColumnCount) - 1;
    }
    partitionSize = new int[1 << exactColumnCount];
    std::memset(partitionSize, 0, sizeof(int) * (1 << exactColumnCount));
    
    S = new std::vector<int>[exactRowCount];
    T = new int[exactRowCount];
    std::memset(T, -1, sizeof(int) * exactRowCount);
    
    std::unordered_map<std::string, int>* mapToHash = new std::unordered_map<std::string, int>[exactColumnCount];
    std::vector<std::vector<int>>* vectorToHash = new std::vector<std::vector<int>>[exactColumnCount];

    std::ifstream in(inputFileName);
    std::string st;
    
    int curRow = 0;
    while (std::getline(in, st))
    {
        st = st + ',';
        std::vector<std::string> v = split(st);
        
        int column = 0;
        for (const std::string& cur: v)
        {
            auto it = mapToHash[column].find(cur);
            if (it == mapToHash[column].end())
            {
                vectorToHash[column].push_back(std::vector<int>(1, curRow));
                mapToHash[column][cur] = (int)mapToHash[column].size();
            }
            else
            {
                vectorToHash[column][it->second].push_back(curRow);
            }
            ++column;
        }
        ++curRow;
    }

    for (int i = 0; i < exactColumnCount; ++i)
    {
        int rest = exactRowCount;
        for (const std::vector<int>& v: vectorToHash[i])
        {
            if (v.size() > 1)
            {
                partition[1 << i].push_back(v);
                ++partitionSize[1 << i];
                rest -= v.size();
            }
        }
        partitionSize[1 << i] += rest;
    }
    
    for (int i = 0; i < exactColumnCount; ++i)
    {
        mapToHash[i].clear();
        vectorToHash[i].clear();
    }
    
    delete[] mapToHash;
    delete[] vectorToHash;
    
    in.close();
}

std::vector<int> Tane::getSubsets(int bin)
{
    std::vector<int> subsets;
    int bin0 = bin;
    while (bin0)
    {
        int bit = lowbit(bin0);
        subsets.push_back(bin - bit);
        bin0 -= bit;
    }
    return subsets;
}

bool Tane::checkAllSubsets(const std::vector<int>& subsets)
{
    for (int subset: subsets)
    {
        if (!available[subset])
        {
            return false;
        }
    }
    return true;
}

void Tane::strippedProduct(int bin, int subl, int subr)
{
    for (int i = 0; i < partition[subl].size(); ++i)
    {
        for (int j: partition[subl][i])
        {
            T[j] = i;
        }
    }
    
    int rest = exactRowCount;
    for (int i = 0; i < partition[subr].size(); ++i)
    {
        for (int j: partition[subr][i])
        {
            if (T[j] != -1)
            {
                S[T[j]].push_back(j);
            }
        }
        for (int j: partition[subr][i])
        {
            if (S[T[j]].size() > 1)
            {
                partition[bin].push_back(S[T[j]]);
                ++partitionSize[bin];
                rest -= S[T[j]].size();
            }
            S[T[j]].clear();
        }
    }
    partitionSize[bin] += rest;
    
    for (int i = 0; i < partition[subl].size(); ++i)
    {
        for (int j: partition[subl][i])
        {
            T[j] = -1;
        }
    }
}

void Tane::computeDependencies(const std::vector<int>& subsets, int bin)
{
    for (int subset: subsets)
    {
        rhsPlus[bin] &= rhsPlus[subset];
    }
    
    strippedProduct(bin, subsets[0], subsets[1]);
    
    int intersect = bin & rhsPlus[bin];
    for (int subset: subsets)
    {
        int A = bin - subset;
        if (A & intersect)
        {
            if (partitionSize[bin] == partitionSize[subset])
            {
                output(subset, A);
                rhsPlus[bin] -= A;
                rhsPlus[bin] &= bin;
            }
        }
    }
}

void Tane::output(int lhs, int rhs)
{
    std::vector<int> part;
    for (int i = 0; i < exactColumnCount; ++i)
    {
        if (lhs & (1 << i))
        {
            part.push_back(i + 1);
        }
    }
    for (int i = 0; i < exactColumnCount; ++i)
    {
        if (rhs & (1 << i))
        {
            part.push_back(i + 1);
            break;
        }
    }
    ans.push_back(part);
}


void Tane::execute()
{
    for (int ccnt = 2; ccnt <= exactColumnCount; ++ccnt)
    {
        for (int i = binL[ccnt]; i <= binR[ccnt]; ++i)
        {
            int bin = binRepresent[i];
            std::vector<int> subsets = getSubsets(bin);
            if (checkAllSubsets(subsets))
            {
                computeDependencies(subsets, bin);
            }
            else
            {
                available[bin] = false;
            }
            
            if (!rhsPlus[bin])
            {
                available[bin] = false;
            }
        }
        
        // a prune method which is proved wrong
        /* for (int i = binL[ccnt]; i <= binR[ccnt]; ++i)
        {
            int bin = binRepresent[i];
            
            if (partitionSize[bin] == exactRowCount)
            {
                int xxx = rhsPlus[bin] & (~bin);
                while (xxx)
                {
                    int A = lowbit(xxx);
                    int yyy = bin;
                    bool flag = true;
                    while (yyy)
                    {
                        int B = lowbit(yyy);
                        if (!(A & rhsPlus[bin + A - B]))
                        {
                            flag = false;
                            break;
                        }
                        yyy -= B;
                    }
                    if (flag)
                    {
                        output(bin, A);
                    }
                    xxx -= A;
                }
                available[bin] = false;
            }
        } */
    }
    
    sort(ans.begin(), ans.end(), cmpByLexico);
    std::cout << "count = " << ans.size() << "\n";
    
    std::ofstream out(outputFileName);
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
