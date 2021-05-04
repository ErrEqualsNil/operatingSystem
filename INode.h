#include<ctime>
#include<vector>
#include "DiskController.h"
#include "Address.h"

const int DIRECT_BLOCK_NUM = 10;
const int INDIRECT_BLOCK_NUM = 1;

const int MAX_FILE_LENGTH = 42;

class INode{
    private:
    DiskController disk;
    int fileLength; //对应文件长度， 以kb为单位
    time_t mtime; // 修改时间
    time_t ctime; // 创建时间
    time_t atime; // 上次访问时间
    int linkNum; // 链接数
    std::vector<Address> directBlockAddress;    
    std::vector<Address> indirectblockAddress;

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

INode::INode(){
    fileLength = 0;
    ctime = mtime = atime = time(0);
    linkNum = 1;
    disk = new DiskController();
}

void INode::createINode(int kbLength){
    //todo
}

void INode::setctime(time_t target){
    ctime = target;
}

void INode::setmtime(time_t target){
    mtime = target;
}

void INode::setatime(time_t target){
    atime = target;
}

void INode::addLinkNum() {
    linkNum ++;
}

void INode::deleteLinkNum() {
    linkNum --;
}

time_t INode::getctime() {
    return ctime;
}

time_t INode::getmtime() {
    return mtime;
}

time_t INode::getatime() {
    return atime;
}

int INode::getFileLength() {
    return fileLength;
}

int INode::getLinkNum() {
    return linkNum;
}

std::vector<Address> INode::getAllBlockAddress() {
    std::vector<Address> ans;
    for(auto addr:directBlockAddress){
        ans.push_back(addr);
    }
    std::string blockContent = disk.readBlock();
    //todo
}
void INode::loadFromString(std::string str) {
    // todo
}
std::string INode::writeToString() {
    //todo
}