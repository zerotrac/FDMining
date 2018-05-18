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
    BruteForce* b = new BruteForce("testdata/test_input.txt");
    b->execute();
    return 0;
}
