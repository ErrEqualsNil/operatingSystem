#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <bitset>
#include <math.h>
#include <ctime>
#include "Components.h"

const int FILE_LENGTH_BYTE = 16 * 1024 * 1024;

const int DIRENT_LENGTH = 1024;
const int DIRENT_AREA_BEGIN = 0;
const int DIRENT_AREA_END = 1024 * 1024 - DIRENT_LENGTH; // 最后一个Dirent开始位置

const int INODE_LENGTH = 128;
const int INODE_AREA_BEGIN = 1024 * 1024;
const int INODE_AREA_END = 2 * 1024 * 1024 - INODE_LENGTH; // 最后一个INode开始位置

const int BLOCK_LENGTH = 1024;
const int BLOCK_AREA_BEGIN = 2 * 1024 * 1024;
const int BLOCK_AREA_END = 16 * 1024 * 1024 - BLOCK_LENGTH; // 最后一个Block开始位置

const int MAX_NUM_UNITS = 16;
const int MAX_FILE_NAME = 23;

//以下是Address实现
std::bitset<24> charToBitset(const char s[3]) {
    std::bitset<24> bits;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 8; ++j)
            bits[i * 8 + j] = ((s[i] >> j) & 1);
    return bits;
}

Address::Address() {
    std::memset(addr, 0, sizeof(addr));
}
int Address::getBlockPos() {
    std::bitset<24> a;
    a = charToBitset(addr);
    int flag = 1;
    int res = 0;
    for (int i = 13; i > 0; i--) {
        res += a[i] * flag;
        flag *= 2;
    }

    return res;
}
int Address::getPos() {
    std::bitset<24> a;
    a = charToBitset(addr);
    int flag = 1;
    int res = 0;
    for (int i = 23; i > 13; i--) {
        res += a[i] * flag;
        flag *= 2;
    }
    return res;
}
void Address::intToAddr(int num) {
    addr[0] = addr[1] = addr[2] = 0;
    for (int i = 0; i < 3; i++) {
        int offset = i * 8;
        addr[i] = (num >> offset) & 0xFF;
    }
}
int Address::AddrToInt() {
    int num = 0;
    for (int i = 0; i < 3; i++) {
        int offset = i * 8;
        num |= (addr[i] & 0xFF) << offset;
    }
    return num;
}

//以下是Block实现

void Block::emptyFill() {
    for(int i = 0;i < BLOCK_LENGTH; i++) {
        content[i] = 0;
    }
}

void Block::randomFill() {
    for(int i = 0;i < BLOCK_LENGTH; i++) {
        content[i] = rand() % 26 + 'A';
    }
}

//以下是Unit实现
Unit::Unit() {
    std::strcpy(fileName, "");
    addr = Address();
    status = isEmpty;
}

//以下是Dirent实现

Dirent::Dirent() {
    Unit emptyUnit = Unit();
    for (int i = 0; i < MAX_NUM_UNITS; i++) {
        units[i] = emptyUnit;
    }
}
Dirent::Dirent(Address cur, Address prev) {
    Unit curUnit = Unit();
    strcpy(curUnit.fileName, ".");
    curUnit.addr = cur;
    curUnit.status = isFolder;
    units[0] = curUnit;

    Unit prevUnit = Unit();
    strcpy(prevUnit.fileName, "..");
    prevUnit.addr = prev;
    prevUnit.status = isFolder;
    units[1] = prevUnit;

    Unit emptyUnit = Unit();
    for (int i = 2; i < MAX_NUM_UNITS; i++) {
        units[i] = emptyUnit;
    }
}

void Dirent::listUnit() {
    for (int i = 0; i < MAX_NUM_UNITS; i++) {
        std::cout << units[i].fileName << "  " << units[i].addr.addr << "  " << units[i].status << std::endl;
    }
}

int Dirent::findUnitIndex(const char *fileName, UnitStatus status) {
    int idx = 0;
    for (; idx < MAX_FILE_NAME; idx++) {
        if (units[idx].status != status) {
            continue;
        }
        if (strcmp(units[idx].fileName, fileName) != 0) {
            continue;
        }
        return idx;
    }
    return -1;
}

//以下是INode实现
INode::INode() {
    ctime = mtime = atime = 0;
    linkNum = 0;
    numDirect = numInDirectBlock = 0;
    fileLength = 0;
}
INode::INode(int fileSize_kb, std::vector<Address>& idleBlockAddrs, DiskController* diskController) {
    ctime = mtime = atime = std::time(0);
    linkNum = 1;
    fileLength = fileSize_kb;
    Address addr;
    if (fileSize_kb <= 10) {
        for (int i = 0; i < fileSize_kb; i++) {
            directBlockAddress[i] = idleBlockAddrs.back();
            std::cout<<"Occupied Block Addr:"<<directBlockAddress[i].AddrToInt()<<std::endl;
            Block b;
            b.randomFill();
            diskController->writeBlock(b, directBlockAddress[i]);
            idleBlockAddrs.pop_back();
        }
        numDirect = fileSize_kb;
        numInDirectBlock = 0;
    }
    else if (fileSize_kb > 10) {
        for (int i = 0; i < 10; i++) {
            directBlockAddress[i] = idleBlockAddrs.back();
            std::cout<<"Occupied Block Addr:"<<directBlockAddress[i].AddrToInt()<<std::endl;
            Block b;
            b.randomFill();
            diskController->writeBlock(b, directBlockAddress[i]);
            idleBlockAddrs.pop_back();
        }

        indirectblockAddress = idleBlockAddrs.back();
        std::cout<<"Occupied Block For Indirect Addr:"<<indirectblockAddress.AddrToInt()<<std::endl;
        idleBlockAddrs.pop_back();

        Address writePos = indirectblockAddress;
        Address blockAddr;

        for (int i = 0; i < (fileSize_kb - 10); i++) {
            addr = idleBlockAddrs.back();
            std::cout<<"Occupied Block Addr:"<<addr.addr<<" "<<addr.AddrToInt()<<std::endl;
            Block b;
            b.randomFill();
            diskController->writeBlock(b, addr);
            idleBlockAddrs.pop_back();
            
            diskController->writeAddress(addr, writePos);
            writePos.intToAddr(writePos.AddrToInt() + 3);
        }
        numDirect = 10;
        numInDirectBlock = fileSize_kb - numDirect;
    }
    std::cout<<"INode Size:"<<sizeof(this)<<std::endl;
}
std::vector<Address> INode::getAllBlockAddress(DiskController *diskController) {
    std::vector<Address> allAddr;
    for (int i = 0; i < numDirect; i++) {
        allAddr.push_back(directBlockAddress[i]);
        std::cout<<"Dirent Block Addrs:"<<directBlockAddress[i].AddrToInt()<<std::endl;
    }
    
    for (int i = 0; i < numInDirectBlock; i++) {
        Address add, val;
        add.intToAddr(indirectblockAddress.AddrToInt() + 3 * i);
        val = diskController->readAddress(add);
        std::cout<<"InDirect Block Addrs:"<<val.AddrToInt()<<std::endl;
        allAddr.push_back(val);
    }
    return allAddr;
}

void INode::cleanINode(std::vector<Address>& idleBlockAddrs, DiskController* disk) {
    fileLength = 0;
    ctime = atime = mtime = 0;
    linkNum = 0;
    numDirect = numInDirectBlock = 0; 
    if (indirectblockAddress.AddrToInt() != 0) {
        disk->cleanBlock(indirectblockAddress);
        idleBlockAddrs.push_back(indirectblockAddress);
    }
    return;
}

//以下是DiskController实现

void DiskController::init() {
    struct stat buffer;
    if (stat(FILE_NAME.c_str(), &buffer) == -1) {
        disk.open(FILE_NAME, std::ios::out | std::ios::binary);

        //填充dirent
        Dirent emptyDirent = Dirent();
        for (int i = DIRENT_AREA_BEGIN; i <= DIRENT_AREA_END; i += DIRENT_LENGTH) {
            disk.seekp(i, std::ios::beg);
            disk.write((char *)&emptyDirent, sizeof(emptyDirent));
        }

        //填充inode
        INode emptyINode = INode();
        for (int i = INODE_AREA_BEGIN; i <= INODE_AREA_END; i += INODE_LENGTH) {
            disk.seekp(i, std::ios::beg);
            disk.write((char *)&emptyINode, sizeof(emptyINode));
        }

        //填充block
        Block emptyBlock;
        emptyBlock.emptyFill();
        for (int i = BLOCK_AREA_BEGIN; i <= BLOCK_AREA_END; i += BLOCK_LENGTH) {
            disk.seekp(i, std::ios::beg);
            disk.write((char *)&emptyBlock, sizeof(emptyBlock));
        }

        //初始化root
        Address rootAddr = Address();
        rootAddr.intToAddr(0);
        Dirent root = Dirent(rootAddr, rootAddr);
        disk.seekp(0, std::ios::beg);
        disk.write((char *)&root, sizeof(root));

        disk.close();
    }
    else {
        std::cout << ".dat Exist!" << std::endl;
    }
    disk.open(FILE_NAME, std::ios::in | std::ios::out | std::ios::binary);
}

INode DiskController::readINode(Address addr) {
    int pos = addr.AddrToInt();
    disk.seekg(pos, std::ios::beg);
    INode target;
    disk.read((char *)&target, sizeof(target));
    return target;
}

Dirent DiskController::readDirent(Address addr) {
    int pos = addr.AddrToInt();
    disk.seekg(pos, std::ios::beg);
    Dirent target;
    disk.read((char *)&target, sizeof(target));
    return target;
}

Block DiskController::readBlock(Address addr) {
    Block block;
    disk.seekg(addr.AddrToInt(), std::ios::beg);
    disk.read((char *)&block, sizeof(block));
    return block;
}

Address DiskController::readAddress(Address addr) {
    Address res;
    disk.seekg(addr.AddrToInt(), std::ios::beg);
    disk.read((char *)&res, sizeof(res));
    return res;
}

void DiskController::writeINode(INode node, Address addr) {
    int pos = addr.AddrToInt();
    if (pos < INODE_AREA_BEGIN || pos > INODE_AREA_END) {
        std::cout << "Write INode Fail, addr: " << pos << std::endl;
        return;
    }
    disk.seekp(pos, std::ios::beg);
    disk.write((char *)&node, sizeof(node));
    return;
}

void DiskController::writeDirent(Dirent dir, Address addr) {
    int pos = addr.AddrToInt();
    if (pos < DIRENT_AREA_BEGIN || pos > DIRENT_AREA_END) {
        std::cout << "Write Dirent Fail, addr: " << addr.AddrToInt() << std::endl;
        return;
    }
    disk.seekp(pos, std::ios::beg);
    disk.write((char *)&dir, sizeof(dir));
    return;
}

void DiskController::writeBlock(Block block, Address addr) {
    int pos = addr.AddrToInt();
    if (pos < BLOCK_AREA_BEGIN || pos > BLOCK_AREA_END) {
        std::cout << "Write Block Fail, addr: " << addr.AddrToInt() << std::endl;
        return;
    }
    disk.seekp(pos, std::ios::beg);
    disk.write((char *)&block, sizeof(block));
    return;
}

void DiskController::cleanBlock(Address addr) {
    Block b;
    b.emptyFill();
    writeBlock(b, addr);
    return;
}

void DiskController::writeAddress(Address address, Address addr) {
    disk.seekp(addr.AddrToInt(), std::ios::beg);
    disk.write((char *)&address, sizeof(address));
}

//以下是Controller实现

Controller::Controller() {
    std::cout<<"Init File System "<<std::endl;
    std::cout<<"Design & Implement: 彭鹏 林师言"<<std::endl;
    Address tmpAddr;
    Dirent tmpDir;
    diskController.init();
    for (int i = DIRENT_AREA_END; i >= DIRENT_AREA_BEGIN; i -= DIRENT_LENGTH) {
        tmpAddr.intToAddr(i);
        tmpDir = diskController.readDirent(tmpAddr);
        if (tmpDir.units[0].status == isEmpty) {
            idleDirentAddrs.push_back(tmpAddr);
        }
    }

    INode tmpINode;
    for (int i = INODE_AREA_END; i >= INODE_AREA_BEGIN; i -= INODE_LENGTH) {
        tmpAddr.intToAddr(i);
        tmpINode = diskController.readINode(tmpAddr);
        if (tmpINode.ctime == 0) {
            idleINodeAddrs.push_back(tmpAddr);
        }
    }

    for (int i = BLOCK_AREA_END; i >= BLOCK_AREA_BEGIN; i -= BLOCK_LENGTH) {
        tmpAddr.intToAddr(i);
        Block b;
        b = diskController.readBlock(tmpAddr);
        if (b.content[0] == 0) {
            idleBlockAddrs.push_back(tmpAddr);
        }
    }

    tmpAddr.intToAddr(0);
    currentDir = diskController.readDirent(tmpAddr);
    rootDir = diskController.readDirent(tmpAddr);
    path_addr.push_back(tmpAddr);
    path_string.push_back("~");
    std::cout<<"Current idleDir Space:"<<idleDirentAddrs.size()<<"; Current idleINode Space:"<<idleINodeAddrs.size()<<"; Current idleBlock Space:"<<idleBlockAddrs.size()<<std::endl;
}

std::string Controller::getPath() {
    std::string str;
    for (int i=0; i < path_addr.size(); i++) {
        std::cout <<path_string[i] << "/";
    }
    return str;
}

void Controller::ls() {
    for (int i = 2; i < MAX_NUM_UNITS; i++) {
        if (currentDir.units[i].status == isEmpty) {
            continue;
        }
        if (currentDir.units[i].status == isFolder) {
            std::cout << currentDir.units[i].fileName << std::endl;
        }
        if (currentDir.units[i].status == isFile) {
            INode file;
            file = diskController.readINode(currentDir.units[i].addr);
            std::cout << currentDir.units[i].fileName << " " << file.fileLength << "KB " << std::endl;
        }
    }
}

void Controller::sum() {
    std::cout << "Remain Store Space : " << idleBlockAddrs.size() << " KB" << std::endl;
}

void split(const std::string &s, std::vector<std::string> &sv, const char *delim = " ") {
    sv.clear();
    char *buffer = new char[s.size() + 1];
    buffer[s.size()] = '\0';
    std::copy(s.begin(), s.end(), buffer);
    char *p = std::strtok(buffer, delim);
    do {
        sv.push_back(p);
    } while ((p = std::strtok(NULL, delim)));
    delete[] buffer;
    return;
}

int Controller::getTmpDir(std::vector<std::string> levels, Dirent curDir, Dirent &res) {
    res = curDir;
    for (std::string level : levels) {
        bool ok = false;
        for (int i = 0; i < MAX_NUM_UNITS; i++) {
            if (strcmp(res.units[i].fileName, level.c_str()) != 0) {
                continue;
            }
            res = diskController.readDirent(res.units[i].addr);
            ok = true;
            break;
        }
        if (!ok) {
            return -1;
        }
    }
    return 0;
}

int Controller::changeDirToDirentAndFilename(std::string dir, Dirent curDir, Dirent &targetDir, std::string &filename) {
    std::vector<std::string> splits;
    split(dir, splits, "/");
    filename = splits.back();
    splits.pop_back();
    if(splits[0] == "~") {
        return Controller::changeDirToDirentAndFilename(dir.substr(2, dir.size() - 2), rootDir, targetDir, filename);
    }
    if (getTmpDir(splits, currentDir, targetDir) == -1) {
        return -1;
    }
    return 0;
}

int Controller::cdOneStep(Dirent& curDir, std::string nextLevel) {
    if(nextLevel == "") {
        return 1;
    }
    if(nextLevel == ".") {
        return 1;
    }
    if(nextLevel == "..") {
        if (path_addr.size() == 0 || path_addr.size() == 1) {
            return 1;
        }
        path_addr.pop_back();
        path_string.pop_back();
        Address curDirAddr = path_addr.back();
        curDir = diskController.readDirent(curDirAddr);
        return 1;
    }
    for(int i = 0; i < MAX_NUM_UNITS; i++) {
        if(curDir.units[i].status == isFolder && curDir.units[i].fileName == nextLevel) {
            Dirent tmp = diskController.readDirent(curDir.units[i].addr);
            std::cout<<"cd into dir, addr: "<<tmp.units[0].addr.AddrToInt()<<std::endl;
            path_string.push_back(curDir.units[i].fileName);
            curDir = tmp;
            path_addr.push_back(curDir.units[0].addr);
            return 1;
        }
    }
    return -1;
}

void Controller::cd(Dirent startDir, std::string targetPath) {
    std::vector<std::string> splits;
    split(targetPath, splits, "/");
    Dirent finishDir = startDir;
    for(std::string level : splits) {
        if(cdOneStep(finishDir, level) == -1) {
            std::cout<<"Invalid Path"<<std::endl;
            return ;
        }
    }
    currentDir = finishDir;
    return;
}

void Controller::touch(Dirent startDir, std::string fileDir, int fileSize) {
    std::string fileName;
    Dirent targetDir;
    if (changeDirToDirentAndFilename(fileDir, startDir, targetDir, fileName) == -1) { 
        std::cout << "Invalid Path!" << std::endl;
        return;
    }
    if (fileSize + 1 > idleBlockAddrs.size() || fileSize > 10 + (1024 / 4)) {
        std::cout << "File Size Invalid, require: "<< fileSize + 1 <<"; Limit: "<<std::min(int(idleBlockAddrs.size()), 10 + (1024 / 4))<<std::endl;
        return;
    }
    if (fileName.length() > MAX_FILE_NAME) {
        std::cout << "File name so long" << std::endl;
        return;
    }

    int emptyPos = MAX_NUM_UNITS;
    for (int i = MAX_NUM_UNITS - 1; i >= 0; i--) {
        if (strcmp(targetDir.units[i].fileName, fileName.c_str()) == 0) {
            std::cout << "Same name file exists!" << std::endl;
            return;
        }
        if (targetDir.units[i].status == isEmpty) {
            emptyPos = i;
        }
    }
    
    if (emptyPos == MAX_NUM_UNITS) {
        std::cout << "Failed to add file to current Dir" << std::endl;
        return;
    }

    INode newFileINode = INode(fileSize, idleBlockAddrs, &diskController);
    Address idleAddr = idleINodeAddrs.back();
    std::cout<<"Occupied INode Addr:"<<idleAddr.AddrToInt()<<std::endl;
    diskController.writeINode(newFileINode, idleAddr);
    idleINodeAddrs.pop_back();
    
    targetDir.units[emptyPos].addr = idleAddr;
    strcpy(targetDir.units[emptyPos].fileName, fileName.c_str());
    targetDir.units[emptyPos].status = isFile;
    
    diskController.writeDirent(targetDir, targetDir.units[0].addr);
    currentDir = diskController.readDirent(currentDir.units[0].addr);

    std::cout<<"Remain Space:"<<idleBlockAddrs.size()<<std::endl;
    return;
}

void Controller::cat(std::string fileDir) {
    Dirent targetDir;
    std::string filename;
    if (changeDirToDirentAndFilename(fileDir, currentDir, targetDir, filename) == -1) {
        std::cout << "Invalid Path!" << std::endl;
    }
    bool ok = false;
    for (int i = 0; i < MAX_FILE_NAME; i++) {
        if (targetDir.units[i].status == isEmpty) {
            continue;
        }
        if (strcmp(targetDir.units[i].fileName, filename.c_str()) != 0) {
            continue;
        }
        if (targetDir.units[i].status == isFolder)
        {
            continue;
        }
        ok = true;
        INode targetFileINode;
        targetFileINode = diskController.readINode(targetDir.units[i].addr);
        std::vector<Address> blockAddrs;
        blockAddrs = targetFileINode.getAllBlockAddress(&diskController);
        for (Address addr : blockAddrs) {
            std::cout<<diskController.readBlock(addr).content;
        }
        std::cout << std::endl;
    }
    if (!ok) {
        std::cout << "Invalid Path" << std::endl;
    }
}

void Controller::cp(std::string srcPath, std::string desPath) {
    Dirent srcDir, desDir;
    std::string srcFilename, desFilename;
    if (changeDirToDirentAndFilename(srcPath, currentDir, srcDir, srcFilename) == -1 ||
        changeDirToDirentAndFilename(desPath, currentDir, desDir, desFilename) == -1) {
        std::cout << "Invalid Path" << std::endl;
        return;
    }
    int srcUnitIdx;
    srcUnitIdx = srcDir.findUnitIndex(srcFilename.c_str(), isFile);
    if (srcUnitIdx == -1) {
        std::cout << "Invalid Path, Src File Not Found" << std::endl;
        return;
    }
    INode src;
    src = diskController.readINode(srcDir.units[srcUnitIdx].addr);
    if (idleBlockAddrs.size() < src.fileLength + 1) {
        std::cout << "Not enough Space For Des File" << std::endl;
        return;
    }
    int emptyPos = MAX_NUM_UNITS;
    for(int i = 0; i < MAX_NUM_UNITS; i++) {
        if (desDir.units[i].status == isEmpty) {
            emptyPos = i;
            break;
        }
    }
    if (emptyPos == MAX_NUM_UNITS) {
        std::cout<<"Not enough empty unit at Des Dir"<<std::endl;
        return;
    }
    if (idleINodeAddrs.size() == 0) {
        std::cout<<"Not enough empty inode for Des Dir"<<std::endl;
        return;
    }
    INode des;
    des.ctime = des.mtime = des.atime = time(0);
    des.fileLength = src.fileLength;
    des.linkNum = 1;
    des.numDirect = src.numDirect;
    des.numInDirectBlock = src.numInDirectBlock;
    std::vector<Address> srcBlocks = src.getAllBlockAddress(&diskController);
    for (int i = 0; i < src.numDirect; i++) {
        Address addr = idleBlockAddrs.back();
        idleBlockAddrs.pop_back();

        Block srcBlock, desB;
        srcBlock = diskController.readBlock(srcBlocks[i]);
        strcpy(desB.content, srcBlock.content);
        diskController.writeBlock(desB, addr);
        des.directBlockAddress[i] = addr;
    }

    Address addrBlockStartPos = idleBlockAddrs.back();
    idleBlockAddrs.pop_back();
    des.indirectblockAddress = addrBlockStartPos;
    for (int i = 0; i < src.numInDirectBlock; i++) {
        Address contentBlock = idleBlockAddrs.back(); // 存内容的block地址
        idleBlockAddrs.pop_back();
        Address contentBlockAddrPos; // 存block地址的pos
        contentBlockAddrPos.intToAddr(addrBlockStartPos.AddrToInt() + i * sizeof(Address));
        
        Block srcBlock;
        srcBlock = diskController.readBlock(srcBlocks[10 + i]);
        Block desB;
        strcpy(desB.content, srcBlock.content);
        diskController.writeBlock(desB, contentBlock);
        diskController.writeAddress(contentBlock, contentBlockAddrPos);
    }
    strcpy(desDir.units[emptyPos].fileName, desFilename.c_str());
    Address desINodeAddr = idleINodeAddrs.back();
    idleINodeAddrs.pop_back();
    diskController.writeINode(des, desINodeAddr);
    std::cout<<"Occupied INode Addr: "<<desINodeAddr.AddrToInt()<<std::endl;
    desDir.units[emptyPos].addr = desINodeAddr;
    desDir.units[emptyPos].status = isFile;
    diskController.writeDirent(desDir, desDir.units[0].addr);
    currentDir = diskController.readDirent(currentDir.units[0].addr);

}

void Controller::deleteFile(INode fileINode, Address INodeAddr) {
    std::vector<Address> blockAddrs = fileINode.getAllBlockAddress(&diskController);
    for (Address blockAddr : blockAddrs) {
        diskController.cleanBlock(blockAddr);
        idleBlockAddrs.push_back(blockAddr);
    }

    fileINode.cleanINode(idleBlockAddrs, &diskController);
    diskController.writeINode(fileINode, INodeAddr);
    idleINodeAddrs.push_back(INodeAddr);

}

void Controller::deleteFolder(Dirent dir, Address direntAddr) {
    for (int i = 2; i < MAX_NUM_UNITS; i++) {
        if (dir.units[i].status == isEmpty) {
            continue;
        }
        if (dir.units[i].status == isFile) {
            std::cout<<"Delete file "<<dir.units[i].fileName<<std::endl;
            deleteFile(diskController.readINode(dir.units[i].addr), dir.units[i].addr);
            dir.units[i].status = isEmpty;
            memset(dir.units[i].fileName, 0, sizeof(dir.units[i].fileName));
        }
        if (dir.units[i].status == isFolder) {
            deleteFolder(diskController.readDirent(dir.units[i].addr), dir.units[i].addr);
            dir.units[i].status = isEmpty;
            memset(dir.units[i].fileName, 0, sizeof(dir.units[i].fileName));
        }
    }
    dir.units[0].status = dir.units[1].status = isEmpty;
    dir.units[0].addr = Address();
    dir.units[1].addr = Address();
    memset(dir.units[0].fileName, 0, sizeof(dir.units[0].fileName));
    memset(dir.units[1].fileName, 0, sizeof(dir.units[1].fileName));
    diskController.writeDirent(dir, direntAddr);
    idleDirentAddrs.push_back(direntAddr);
    currentDir = diskController.readDirent(currentDir.units[0].addr);
}

void Controller::del(Dirent startDir, std::string fileDir) {
    Dirent targetDir;
    std::string name;
    if(-1 == changeDirToDirentAndFilename(fileDir, currentDir, targetDir, name)) {
        std::cout<<"Invalid Path"<<std::endl;
        return ;
    }
    bool ok = false;
    for (int i = 2; i < MAX_NUM_UNITS; i++) {
        if (targetDir.units[i].status == isEmpty) {
            continue;
        }
        if (strcmp(targetDir.units[i].fileName, name.c_str()) == 0) {
            ok = true;
            if (targetDir.units[i].status == isFile) {
                std::cout<<"Delete INode: "<<targetDir.units[i].addr.AddrToInt()<<std::endl;
                deleteFile(diskController.readINode(targetDir.units[i].addr), targetDir.units[i].addr);
                targetDir.units[i].status = isEmpty;
                strcpy(targetDir.units[i].fileName, "");
            }
            else if (targetDir.units[i].status == isFolder) {
                deleteFolder(diskController.readDirent(targetDir.units[i].addr), targetDir.units[i].addr);
                targetDir.units[i].status = isEmpty;
                strcpy(targetDir.units[i].fileName, "");
            }
        }
    }
    if (!ok) {
        std::cout << "Invalid Path!" << std::endl;
        return ;
    }
    diskController.writeDirent(targetDir, targetDir.units[0].addr);
    currentDir = diskController.readDirent(currentDir.units[0].addr);
}
void Controller::mkdir(std::string folderDir) {
    std::string folderName;
    Dirent targetDir;
    if (changeDirToDirentAndFilename(folderDir, currentDir, targetDir, folderName) == -1) {
        std::cout << "Invalid Path!" << std::endl;
        return;
    }
    int emptyPos = MAX_NUM_UNITS;
    for (int i = MAX_NUM_UNITS - 1; i >= 0; i--) {
        if (targetDir.units[i].status == isEmpty)
        {
            emptyPos = i;
        }
        if (strcmp(targetDir.units[i].fileName, folderName.c_str()) == 0)
        {
            std::cout << "Same name folder exists!" << std::endl;
            return;
        }
    }
    if (emptyPos == MAX_NUM_UNITS) {
        std::cout << "Failed to create folder to current Dir" << std::endl;
        return;
    }
    Address newDirAddr = idleDirentAddrs.back();
    Dirent newFolderDir = Dirent(newDirAddr, targetDir.units[0].addr);
    targetDir.units[emptyPos].addr = newDirAddr;
    strcpy(targetDir.units[emptyPos].fileName, folderName.c_str());
    targetDir.units[emptyPos].status = isFolder;
    diskController.writeDirent(newFolderDir, newDirAddr);
    idleDirentAddrs.pop_back();
    diskController.writeDirent(targetDir, targetDir.units[0].addr);
    currentDir = diskController.readDirent(currentDir.units[0].addr);
    return;
}
void Controller::exit() {
    diskController.writeDirent(currentDir, currentDir.units[0].addr);
    std::cout<<"Welcome Back!"<<std::endl;
}

int Controller::waitForCommand() {
    std::cout<<getPath()<<" >";
    std::string command;
    getline(std::cin, command);
    std::vector<std::string> parts;
    split(command, parts, " ");
    if (parts[0] == "touch") {
        if(parts.size() != 3) {
            std::cout<<"Invalid number of args: "<<parts.size() - 1<<" required:2"<<std::endl;
            return 0;
        }
        int fileSizeParam = atoi(parts[2].c_str());
        touch(currentDir, parts[1], fileSizeParam);
    }
    else if (parts[0] == "mkdir") {
        if(parts.size() != 2) {
            std::cout<<"Invalid number of args: "<<parts.size() - 1<<" required:1"<<std::endl;
            return 0;
        }
        mkdir(parts[1]);
    }
    else if (parts[0] == "del") {
        if(parts.size() != 2) {
            std::cout<<"Invalid number of args: "<<parts.size() - 1<<" required:1"<<std::endl;
            return 0;
        }
        del(currentDir, parts[1]);
    }
    else if (parts[0] == "cd") {
        if(parts.size() != 2) {
            std::cout<<"Invalid number of args: "<<parts.size() - 1<<" required:1"<<std::endl;
            return 0;
        }
        cd(currentDir, parts[1]);
    }
    else if (parts[0] == "ls") {
        if(parts.size() != 1) {
            std::cout<<"Invalid number of args: "<<parts.size() - 1<<" required:0"<<std::endl;
            return 0;
        }
        ls();
    }
    else if (parts[0] == "cp") {
        if(parts.size() != 3) {
            std::cout<<"Invalid number of args: "<<parts.size() - 1<<" required:2"<<std::endl;
            return 0;
        }
        cp(parts[1], parts[2]);
    }
    else if (parts[0] == "sum") {
        if(parts.size() != 1) {
            std::cout<<"Invalid number of args: "<<parts.size() - 1<<" required:0"<<std::endl;
            return 0;
        }
        sum();
    }
    else if (parts[0] == "cat") {
        if(parts.size() != 2) {
            std::cout<<"Invalid number of args: "<<parts.size() - 1<<" required:1"<<std::endl;
            return 0;
        }
        cat(parts[1]);
    }
    else if (parts[0] == "exit") {
        if(parts.size() != 1) {
            std::cout<<"Invalid number of args: "<<parts.size() - 1<<" required:0"<<std::endl;
            return 0;
        }
        exit();
        return -1;
    }
    return 1;
}