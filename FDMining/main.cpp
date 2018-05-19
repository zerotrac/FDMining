#include <cstring>
#include <iostream>

#include "Tane.h"

inline void optimize()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
}

int main(int argc, char** argv)
{
    clock_t ta = clock();
    optimize();
    
    if (argc < 2)
    {
        std::cout << "Argument Error!\n";
        return 0;
    }
    
    if (strcmp(argv[1], "small") == 0)
    {
        Tane* t = new Tane("testdata/data_small.txt", "testdata/fd_small.txt", 0);
        t->execute();
    }
    else if (strcmp(argv[1], "large") == 0)
    {
        Tane* t = new Tane("testdata/data_large.txt", "testdata/fd_large.txt", 1);
        t->execute();
    }
    else
    {
        std::cout << "Argument Error!\n";
        return 0;
    }
    
    clock_t tb = clock();
    std::cout << "Duration Time: " << (double)(tb - ta) / CLOCKS_PER_SEC << " s\n";
    return 0;
}
