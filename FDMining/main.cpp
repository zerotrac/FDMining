#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "BruteForce.h"

inline void optimize()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
}

int main()
{
    clock_t ta = clock();
    BruteForce* b = new BruteForce("testdata/data.txt");
    // BruteForce* b = new BruteForce("testdata/test_input.txt");
    b->execute();
    clock_t tb = clock();
    std::cout << "Duration Time: " << (double)(tb - ta) / CLOCKS_PER_SEC << " s\n";
    return 0;
}
