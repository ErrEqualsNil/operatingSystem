Operating System Course Design

空间划分
Address 3byte = 24 bits
Unit 28byte 分为24byte的文件名，3byte地址， 1byte isFile
Dirent 448byte -> 512byte 分为16 * Unit, 共2048个，占1MB
INode 4+24+4+30+3+4+4 = 73byte -> 128byte, 共8 * 1024个，占1MB
