#include <iostream>

using namespace std;

int main() {
    string str = "abc";
    bitset<24> b(stoi(str));
    cout<<b<<endl;
    return 0;
}