#ifndef Debug_h
#define Debug_h

#include <vector>
#include <iostream>

class Debug
{
public:
    template<class T>
    static void printVector(std::vector<T>& v)
    {
        std::cout << "[ ";
        for (const T& element: v) std::cout << element << " ";
        std::cout << "]\n";
    }
    
    static void debug(const std::string& st)
    {
        std::cout << st << "\n";
    }
};

#endif
