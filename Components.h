#include<bitset>
#include<vector>
#include<iostream>
#include<fstream>

enum UnitStatus {
    isFile,
    isFolder,
    isEmpty
};

class Address{
    // 地址，24位，前14位表示block， 后10位表示block中位置
    public:
    char addr[3]; // 每位char表示1byte = 8位， 使用时注意转回来处理
    Address(); //构造函数
    int getBlockPos(); // 获取地址对应的块的序号（前14位组成的int）
    int getPos(); // 获取地址对应的块内序号 （后10位组成的int）
    void intToAddr(); // 将byte序号转换为地址， eg. 序号1025 -> block 1 pos 1
};

class Unit{
    // Dirent使用的单位，包含名称及对应的inode/dirent首位地址
    public:
    char fileName[24]; // 限长24
    Address addr;
    UnitStatus status;
};

class Dirent{
    public:
    Unit units[16]; 
    Dirent(); //需要初始化units的status为isEmpty , 设置第0个为“.”目录; 第1个为".."目录
    void listUnit();
    void addNewUnit(Unit newUnit);
    void deleteUnit(std::string unitName);
};

class INode{
    public:
    int fileLength; //对应文件长度， 以kb为单位
    time_t mtime; // 修改时间
    time_t ctime; // 创建时间
    time_t atime; // 上次访问时间
    int linkNum; // 链接数
    Address directBlockAddress[10]; //如果指向0块0个，则认为是空
    int numDirect;
    Address indirectblockAddress; //如果指向0块0个，则认为是空
    int numInDirectBlock;
    
    void createINode(int kbLength);
    void addLinkNum();
    void deleteLinkNum();
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