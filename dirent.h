#include "Address.h"
#include "INode.h"
#include <vector>

class Unit{
    public:
    std::string fileName;
    Address addr;
    bool isFile;
};

class Dirent{
    private:
    std::vector<Unit> units;
    
    public:
    Dirent();
    void listUnit();
    void addNewUnit(Unit newUnit);
    void deleteUnit(std::string unitName);
};