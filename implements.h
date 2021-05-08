#include "Components.h"
#include <iostream>
#include <fstream>
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