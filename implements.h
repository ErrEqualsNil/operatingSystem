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

//以下是Address实现

Address::Address(){
    memset(addr, 0, sizeof(addr));
}
int Address::getBlockPos(){

}
int Address::getPos(){

}
void Address::intToAddr(int num){

}
int Address::AddrToInt(){

}


//以下是Unit实现
Unit::Unit(){
    memset(fileName, 0, sizeof(fileName));
    addr = Address();
    status = isEmpty;
}


//以下是Dirent实现

Dirent::Dirent(){
    Unit emptyUnit = Unit();
    for(int i=0; i<MAX_NUM_UNITS ;i++){
        units[0] = emptyUnit;
    }
}


//以下是INode实现




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
        disk.seekp(0, std::ios::beg);
        Address rootAddr = Address();
        rootAddr.intToAddr(0);
        Dirent root = Dirent(rootAddr, rootAddr);
        root.units[0].status = isFolder;
        disk.write((char*)&root, sizeof(root));
        
        disk.close();
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
    char fillCharacter = rand();
    for(int i=addr.AddrToInt();i<addr.AddrToInt() + BLOCK_LENGTH; i++){
        disk.write(&fillCharacter, sizeof(fillCharacter));
    }
    return;
}


//以下是Controller实现
