#include "Components.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/stat.h>

const int FILE_LENGTH_BYTE = 16 * 1024 * 1024;
const int DIRENT_AREA_BEGIN = 0;
const int DIRENT_AREA_END = 1024 * 1024 - 512; // 最后一个Dirent开始位置
const int DIRENT_LENGTH = 512;
const int INODE_AREA_BEGIN = 1024 * 1024;
const int INODE_AREA_END = 2 * 1024 * 1024 - 64; // 最后一个INode开始位置
const int INODE_LENGTH = 64;
const int BLOCK_AREA_BEGIN = 2 * 1024 * 1024;
const int BLOCK_AREA_END = 16 * 1024 * 1024 - 1024; // 最后一个Block开始位置
const int BLOCK_LENGTH = 1024;
const int MAX_NUM_UNITS = 16;
const int MAX_FILE_NAME = 23;

//以下是Address实现

Address::Address(){
    memset(addr, 0, sizeof(addr));
}
int Address::getBlockPos(){
    return 0;
}
int Address::getPos(){
    return 0;
}
void Address::intToAddr(int num){
    addr[0] = addr[1] = addr[2] = 0;
}
int Address::AddrToInt(){
    return 0;
}


//以下是Unit实现
Unit::Unit(){
    strcpy(fileName, "");
    addr = Address();
    status = isEmpty;
}


//以下是Dirent实现

Dirent::Dirent(){
    Unit emptyUnit = Unit();
    for(int i=0; i<MAX_NUM_UNITS ;i++){
        units[i] = emptyUnit;
    }
}
Dirent::Dirent(Address cur, Address prev){
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
    for (int i=2; i<MAX_NUM_UNITS; i++){
        units[i] = emptyUnit;
    }
}


//以下是INode实现
INode::INode(){
    ctime = mtime = atime = 0;
    linkNum = 0;
    numDirect = numInDirectBlock = 0;
    fileLength = 0;
}
INode::INode(int fileSize_kb){
    
}


//以下是DiskController实现

DiskController::DiskController() {
    struct stat buffer;
    if (stat(FILE_NAME.c_str(), &buffer) == -1){
        disk.open(FILE_NAME, std::ios::out | std::ios::binary);
        
        //填充dirent
        Dirent emptyDirent = Dirent();
        for(int i=DIRENT_AREA_BEGIN; i<= DIRENT_AREA_END; i += DIRENT_LENGTH){
            disk.seekp(i, std::ios::beg);
            disk.write((char*)&emptyDirent, sizeof(emptyDirent));
        }
        
        //填充inode
        INode emptyINode = INode();
        for(int i=INODE_AREA_BEGIN; i<= INODE_AREA_END; i += INODE_LENGTH){
            disk.seekp(i, std::ios::beg);
            disk.write((char*)&emptyINode, sizeof(emptyINode));
        }
        
        //填充block
        const char FILL_CHAR = 0;
        for(int i = BLOCK_AREA_BEGIN; i < FILE_LENGTH_BYTE; i++){
            disk.write(&FILL_CHAR, sizeof(FILL_CHAR));
        }
        
        //初始化root
        Address rootAddr = Address();
        rootAddr.intToAddr(0);
        Dirent root = Dirent(rootAddr, rootAddr);
        disk.seekp(0, std::ios::beg);
        disk.write((char*)&root, sizeof(root));
        
        disk.close();
    }
    else {
        std::cout<<".dat Exist!"<<std::endl;
    }
    disk.open(FILE_NAME, std::ios::in | std::ios::out | std::ios::binary);
}

INode DiskController::readINode(Address addr){
    int pos = 1024 * addr.getBlockPos() + addr.getPos();
    disk.seekg(pos, std::ios::beg);
    INode target;
    disk.read((char*)&target, sizeof(target));
    return target;
}

Dirent DiskController::readDirent(Address addr){
    int pos = 1024 * addr.getBlockPos() + addr.getPos();
    disk.seekg(pos, std::ios::beg);
    Dirent target;
    disk.read((char*)&target, sizeof(target));
    return target;
}

std::string DiskController::readBlock(Address addr){
    if(addr.getPos() != 0){
        return "";
    }
    int pos = 1024 * addr.getBlockPos();
    std::string target;
    char character;
    disk.seekg(pos, std::ios::beg);
    for(int i = pos; i < pos + 1024; i++){
        disk.read(&character, sizeof(character));
        target = target + character;
    }
    return target;
}

void DiskController::writeINode(INode node, Address addr){
    int blockpos = addr.getBlockPos();
    int pos = addr.getPos();
    if (addr.AddrToInt() < INODE_AREA_BEGIN 
        || addr.AddrToInt() > INODE_AREA_END
        || pos % INODE_LENGTH != 0) {  
        std::cout<<"Write INode Fail, addr: "<<addr.AddrToInt()<<std::endl;
        return;
    }
    disk.seekp(addr.AddrToInt(), std::ios::beg);
    disk.write((char*)&node, sizeof(node));
    return;
}

void DiskController::writeDirent(Dirent dir, Address addr){
    int blockpos = addr.getBlockPos();
    int pos = addr.getPos();
    if (addr.AddrToInt() < DIRENT_AREA_BEGIN 
        || addr.AddrToInt() > DIRENT_AREA_END
        || pos % DIRENT_LENGTH != 0) {  
        std::cout<<"Write Dirent Fail, addr: "<<addr.AddrToInt()<<std::endl;
        return;
    }
    disk.seekp(addr.AddrToInt(), std::ios::beg);
    disk.write((char*)&dir, sizeof(dir));
    return;
}

void DiskController::writeBlock(Address addr){
    if (addr.AddrToInt() < BLOCK_AREA_BEGIN 
        || addr.AddrToInt() > BLOCK_AREA_END) {  
        std::cout<<"Write Block Fail, addr: "<<addr.AddrToInt()<<std::endl;
        return;
    }
    char fillCharacter = 'p';
    for(int i=addr.AddrToInt();i<addr.AddrToInt() + BLOCK_LENGTH; i++){
        disk.write(&fillCharacter, sizeof(fillCharacter));
    }
    return;
}


//以下是Controller实现

Controller::Controller(){
    Address tmpAddr;

    Dirent tmpDir;
    for(int i=DIRENT_AREA_BEGIN ;i < DIRENT_AREA_END; i+= DIRENT_LENGTH){
        tmpAddr.intToAddr(i);
        tmpDir = diskController.readDirent(tmpAddr);
        if(tmpDir.units[0].status == isEmpty){
            idleDirentAddrs.push_back(tmpAddr);
        }
    }

    INode tmpINode;
    for(int i=INODE_AREA_BEGIN ;i < INODE_AREA_END; i+= INODE_LENGTH){
        tmpAddr.intToAddr(i);
        tmpINode = diskController.readINode(tmpAddr);
        if(tmpINode.ctime == 0){
            idleINodeAddrs.push_back(tmpAddr);
        }
    }

    for(int i=BLOCK_AREA_BEGIN ;i < BLOCK_AREA_END; i+= BLOCK_LENGTH){
        tmpAddr.intToAddr(i);
        char firstPos;
        diskController.disk.read(&firstPos, sizeof(char));
        if(firstPos == 0){
            idleBlockAddrs.push_back(tmpAddr);
        }
    }

    tmpAddr.intToAddr(0);
    currentDir = diskController.readDirent(tmpAddr);
    rootDir = diskController.readDirent(tmpAddr);
    path.push_back("~");
}

std::string Controller::getPath(){
    std::string str;
    for(std::string folderName : path){
        std::cout<<folderName<<" > "<<std::endl;
    }
    return str;
}

void Controller::ls(){
    std::cout<<"Current Path: "<<getPath()<<std::endl;
    for(int i=0; i<MAX_NUM_UNITS; i++){
        if(currentDir.units[i].status == isEmpty){
            continue;
        }
        if(currentDir.units[i].status == isFolder){
            std::cout<<currentDir.units[i].fileName<<std::endl;
        }
        if(currentDir.units[i].status == isFile){
            INode file;
            file = diskController.readINode(currentDir.units[i].addr);
            std::cout<<currentDir.units[i].fileName<<" "<<file.fileLength<<"KB "<<std::endl;
        }
    }
}

void Controller::sum(){
    std::cout<<"Remain Store Space : "<<idleBlockAddrs.size()<<" KB"<<std::endl;
}

void split(const std::string& s,
    std::vector<std::string>& sv,
                  const char* delim = " ") {
    sv.clear();                                 
    char* buffer = new char[s.size() + 1];      
    buffer[s.size()] = '\0';
    std::copy(s.begin(), s.end(), buffer);      
    char* p = std::strtok(buffer, delim);       
    do {
        sv.push_back(p);                        
    } while ((p = std::strtok(NULL, delim)));   
    delete[] buffer;
    return;
}

int Controller::getTmpDir(std::vector<std::string> levels, Dirent curDir, Dirent& res){
    res = curDir;
    for(std::string level : levels){
        bool ok = false;
        for(int i=0; i<MAX_NUM_UNITS; i++){
            if(strcmp(res.units[i].fileName, level.c_str()) != 0){
                continue;
            }
            res = diskController.readDirent(res.units[i].addr);
            ok = true;
            break;
        }
        if(!ok){
            return -1;
        }
    }
    return 0;
} 

void Controller::cd(Dirent startDir, std::string targetPath){
    if(targetPath == ""){
        return ;
    }
    if(targetPath[0] == '~'){
        Controller::cd(rootDir, targetPath.substr(2, targetPath.size() - 2));
    }
    std::vector<std::string> splits;
    split(targetPath, splits, "/");
    Dirent finishDir = startDir;
    getTmpDir(splits, startDir, finishDir);
    currentDir = finishDir;
    return ;
}

void Controller::touch(std::string fileDir, int fileSize){
    std::vector<std::string> splits;
    split(fileDir, splits, "/");
    std::string fileName = splits.back();
    if(fileName.length() > MAX_FILE_NAME){
        std::cout<<"File name so long"<<std::endl;
        return ;
    }
    splits.pop_back();
    Dirent targetDir;
    getTmpDir(splits, currentDir, targetDir);
    INode newFileINode = INode(fileSize);
    Address idleAddr = idleINodeAddrs.back();
    idleINodeAddrs.pop_back();

    bool ok = false;
    for(int i=0; i<MAX_NUM_UNITS; i++){
        if (targetDir.units[i].status == isEmpty){
            targetDir.units[i].addr = idleAddr;
            strcpy(targetDir.units[i].fileName, fileName.c_str());
            targetDir.units[i].status = isFile;
            diskController.writeINode(newFileINode, idleAddr);
            ok = true;
            break;
        }
    }
    if(!ok){
        std::cout<<"Failed to add file to current Dir"<<std::endl;
    }
    return ;
}
