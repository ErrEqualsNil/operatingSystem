#include "Components.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>

const int FILE_LENGTH_BYTE = 16 * 1024 * 1024;
DiskController::DiskController() {
    struct stat buffer;
    if (stat(FILE_NAME.c_str(), &buffer) == -1){
        disk.open(FILE_NAME, std::ios::out | std::ios::binary);
        const char FILL_CHAR = 0;
        for(int i = 0; i < FILE_LENGTH_BYTE; i++){
            disk.write(&FILL_CHAR, sizeof(FILL_CHAR));
        }
        disk.close();
    }
    else {
        std::cout<<"File Exist!";
    }
}