#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "TaneOptimized.h"

TaneOptimized::TaneOptimized(const std::string& _inputFileName, const std::string& _outputFileName, int smallOrLarge)
{
    inputFileName = _inputFileName;
    outputFileName = _outputFileName;
    
    if (smallOrLarge)
    {
        exactRowCount = ROW_COUNT_LARGE;
        exactColumnCount = COLUMN_COUNT_LARGE;
    }
    else
    {
        exactRowCount = ROW_COUNT_SMALL;
        exactColumnCount = COLUMN_COUNT_SMALL;
    }
    
    clock_t ta = clock();
    prepareLayers();
    clock_t tb = clock();
    prepareData();
    clock_t tc = clock();
    double xx = (double)(tb - ta) / CLOCKS_PER_SEC;
    double yy = (double)(tc - tb) / CLOCKS_PER_SEC;
    std::cout << "prepare layers: " << xx << "\n";
    std::cout << "prepare data: " << yy << "\n";
    
    presubl = -1;
}

TaneOptimized::~TaneOptimized()
{
    // do nothing
}


void TaneOptimized::prepareLayers()
{
    binRepresent.clear();
    for (int i = 0; i < (1 << exactColumnCount); ++i)
    {
        binRepresent.push_back(i);
    }
    std::sort(binRepresent.begin(), binRepresent.end(), cmpBySize);
    
    std::memset(available, true, sizeof(available));
    
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

void TaneOptimized::prepareData()
{
    FILE* input = fopen(inputFileName.c_str(), "r");
    // setvbuf(input, new char[1 << 25], _IOFBF, 1 << 25);
    
    for (int i = 0; i < (1 << exactColumnCount); ++i)
    {
        rhsPlus[i] = (1 << exactColumnCount) - 1;
    }
    
    std::memset(T, -1, sizeof(T));

    std::vector<std::string> v;
    while (fgets(buf, 1 << 20, input))
    {
        size_t sz = strlen(buf);
        if (buf[sz - 1] == '\n') --sz;
        std::string st = std::string(buf, sz) + ',';
        // std::string st = std::string(buf) + ',';
        split(v, st);
        
        int column = 0;
        for (const std::string& cur: v)
        {
            data[column].push_back(cur);
            ++column;
        }
    }
    
    for (int column = 0; column < exactColumnCount; ++column)
    {
        int curRow = 0;
        for (const std::string& cur: data[column])
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
            ++curRow;
        }
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
    
    fclose(input);
}

void TaneOptimized::getSubsets(std::vector<int>& subsets, int bin)
{
    subsets.clear();
    int bin0 = bin;
    while (bin0)
    {
        int bit = lowbit(bin0);
        subsets.push_back(bin - bit);
        bin0 -= bit;
    }
}

bool TaneOptimized::checkAllSubsets(const std::vector<int>& subsets)
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

bool TaneOptimized::strippedProduct(int bin, int subl, int subr, int presubl)
{
    if (partitionSize[subl] == exactRowCount || partitionSize[subr] == exactRowCount)
    {
        partitionSize[bin] = exactRowCount;
        return false;
    }
    
    if (subl != presubl)
    {
        std::memset(T, -1, sizeof(T));
        int i = 0;
        for (const std::vector<int>& part: partition[subl])
        {
            for (int j: part)
            {
                T[j] = i;
            }
            ++i;
        }
    }
    
    int rest = exactRowCount;
    for (const std::vector<int>& part: partition[subr])
    {
        for (int j: part)
        {
            if (T[j] != -1)
            {
                S[T[j]].push_back(j);
            }
        }
        for (int j: part)
        {
            if (T[j] != -1)
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
    }
    partitionSize[bin] += rest;
    return true;
}

void TaneOptimized::computeDependencies(const std::vector<int>& subsets, int bin)
{
    int big = -1;
    for (int subset: subsets)
    {
        rhsPlus[bin] &= rhsPlus[subset];
        if (subset != subsets[0] && (big == -1 || partitionSize[subset] > partitionSize[big]))
        {
            big = subset;
        }
    }
    
    bool changed = strippedProduct(bin, subsets[0], big, presubl);
    if (changed) presubl = subsets[0];
    
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

void TaneOptimized::output(int lhs, int rhs)
{
    part.clear();
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


void TaneOptimized::execute()
{
    for (int ccnt = 2; ccnt <= exactColumnCount; ++ccnt)
    {
        clock_t a1 = clock();
        int cntcnt = 0;
        
        for (int i = binL[ccnt]; i <= binR[ccnt]; ++i)
        {
            int bin = binRepresent[i];
            getSubsets(subsets, bin);
            if (checkAllSubsets(subsets))
            {
                ++cntcnt;
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

        clock_t a2 = clock();
        double dura = (double)(a2 - a1) / CLOCKS_PER_SEC;
        std::cout << "do partition " << cntcnt << " times\n";
        std::cout << "duration = " << dura << "\n";
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
