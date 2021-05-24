#include <bitset>
#include <vector>
#include <iostream>
#include <fstream>

enum UnitStatus
{
    isFile,
    isFolder,
    isEmpty
};

class Address;
class Unit;
class Dirent;
class INode;
class DiskController;
class Controller;

class Address
{
    // 地址，24位，前14位表示block， 后10位表示block中位置
public:
    char addr[3];            // 每位char表示1byte = 8位， 使用时注意转回来处理
    Address();               //构造函数
    int getBlockPos();       // 获取地址对应的块的序号（前14位组成的int）
    int getPos();            // 获取地址对应的块内序号 （后10位组成的int）
    void intToAddr(int num); // 将byte序号转换为地址， eg. 序号1025 -> block 1 pos 1
    int AddrToInt();         // 将地址转为byte序号
};

class Unit
{
    // Dirent使用的单位，包含名称及对应的inode/dirent首位地址
public:
    Unit();
    char fileName[24]; // 限长24
    Address addr;
    UnitStatus status;
};

class Dirent
{
public:
    Unit units[16];
    Dirent();                          //默认初始化， 将units填充并空置，status设为isEmpty
    Dirent(Address cur, Address prev); //初始化为新的目录，cur为该dirent起始地址， prev为上级dirent起始地址，并初始化unit[0]为 . ; unit[1]为 ..
    void listUnit();
    int findUnitIndex(const char *fileName, UnitStatus status);
};

class INode
{
public:
    int fileLength;                 //对应文件长度， 以kb为单位
    time_t mtime;                   // 修改时间
    time_t ctime;                   // 创建时间
    time_t atime;                   // 上次访问时间
    int linkNum;                    // 链接数 -> update: 好像不需要链接，对于复制关系直接把数据块Addr同步过去算了，做链接有点复杂（但是别删这个变量，免得影响数据大小规划）
    Address directBlockAddress[10]; //如果指向0块0个，则认为是空
    int numDirect;
    Address indirectblockAddress; //如果指向0块0个，则认为是空
    int numInDirectBlock;
    INode();                                                                                 //空INode， 所有值置0
    INode(int kbLength, std::vector<Address> &idleBlockAddrs, DiskController *DiskController); //建立具有kbLength长度的INode
    std::vector<Address> getAllBlockAddress(DiskController *diskController);
    void cleanINode(std::vector<Address>& idleINodeAddrs, DiskController* disk);
};

class DiskController
{
public:
    std::fstream disk;
    const std::string FILE_NAME = "output/disk.dat";
    void init();
    INode readINode(Address addr);
    Dirent readDirent(Address addr);
    std::string readBlock(Address addr);
    Address readAddress(Address addr);
    void writeINode(INode node, Address addr);
    void writeDirent(Dirent dir, Address addr);
    void writeBlock(Address addr);
    void cleanBlock(Address addr);
    void writeAddress(Address address, Address addr); // 将address写入addr地址
};

class Controller
{
public:
    Dirent currentDir;
    Dirent rootDir;
    std::vector<std::string> path;
    DiskController diskController;
    std::vector<Address> idleDirentAddrs;
    std::vector<Address> idleINodeAddrs;
    std::vector<Address> idleBlockAddrs;
    Controller();
    std::string getPath();
    int getTmpDir(std::vector<std::string> levels, Dirent curDir, Dirent &res);
    int changeDirToDirentAndFilename(std::string dir, Dirent curDir, Dirent &targetDir, std::string &filename);
    void touch(std::string fileDir, int fileSize);
    void mkdirp(std::string folderDir);
    void mkdir(std::string folderDir);
    void del(std::string fileDir);
    void deleteFile(INode fileINode, Address INodeAddr);
    void deleteFolder(Dirent dir, Address direntAddr);
    void cd(Dirent startDir, std::string targetDir);
    void ls();
    void cp(std::string srcPath, std::string desPath);
    void sum();
    void cat(std::string fileDir); 
    void exit();
    int waitForCommand();
};