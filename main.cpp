#include <iostream>
#include <bitset>
#include <vector>
#include <fstream>
#include "implements.cpp"

using namespace std;

int main() {
    Controller c;
    int total, cur;
    total = cur = 0;
    while(true){
        int res;
        res = c.waitForCommand();
        if (res == -1){
            break;
        }
        total += 1;
        cur += res;
    }
    std::cout<<"Successfully Execute: "<< cur <<" / Total: "<< total <<endl;
    return 0;
}