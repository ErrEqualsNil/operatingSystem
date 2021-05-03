#include <iostream>
#include <fstream>
#include "Address.h"
#include "INode.h"
#include "dirent.h"

const std::string DISK_DIR = "./Disk";

class DiskController{
    private:
    std::fstream disk;
    public:
    DiskController();
    INode readINode(Address addr);
    Dirent readDirent(Address addr);
    std::string readBlock(Address addr);
    void writeINode(INode node, Address addr);
    void writeDirent(Dirent dir, Address addr);
    void writeBlock(Address addr);
};

DiskController::DiskController() {
    disk.open(DISK_DIR, std::ios::in || std::ios::out);
}