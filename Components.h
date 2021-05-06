#include<bitset>
#include<vector>
#include<iostream>
#include<fstream>

class Address{
    // 地址，24位，前14位表示block， 后10位表示block中位置
    private:
    std::bitset<24> addr;
    public:
    Address();
    std::string addressToString();
    bool stringToAddress(std::string str);
};

class Unit{
    // Dirent使用的单位，包含名称及对应的inode/dirent首位地址
    public:
    const int 
    string fileName;
    Address addr;
    bool isFile;
};

class Dirent{
    private:
    std::vector<Unit> units;
    int numUnits;
    
    public:
    Dirent();
    void listUnit();
    void addNewUnit(Unit newUnit);
    void deleteUnit(std::string unitName);
};

class INode{
    private:
    int fileLength; //对应文件长度， 以kb为单位
    time_t mtime; // 修改时间
    time_t ctime; // 创建时间
    time_t atime; // 上次访问时间
    int linkNum; // 链接数
    std::vector<Address> directBlockAddress;
    int numDirectBlock;
    std::vector<Address> indirectblockAddress;
    int numIndirectBlock;

    public:
    INode();
    void createINode(int kbLength);
    void setctime(time_t target_time); 
    void setmtime(time_t target_time); 
    void setatime(time_t target_time); 
    void addLinkNum();
    void deleteLinkNum();
    time_t getctime();
    time_t getmtime();
    time_t getatime();
    int getFileLength();
    int getLinkNum();
    std::vector<Address> getAllBlockAddress();
    void loadFromString(std::string str);
    std::string writeToString();
};

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