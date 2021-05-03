#include "Address.h"
#include "dirent.h"
#include "INode.h"
#include <iostream>
#include <vector>

class Controller {
    private:
    Dirent currentDir;
    std::vector<Address> idleDirentAddr;
    std::vector<Address> idleINodeAddr;
    std::vector<Address> idleStoreAddr;
    public:
    Controller();
    void initDisk();  
    void writeDisk();
    void touch(std::string fileDir, int fileSize_kb);
    void del(std::string fileDir);
    void cd(std::string targetDir);
    void ls();
    void cp(std::string srcDir, std::string desDir);
    void sum();
    void cat(std::string fileDir);
};