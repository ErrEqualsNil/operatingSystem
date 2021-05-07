Operating System Course Design

空间划分

Address 3 byte = 24 bits

Unit 32 byte 分为28 byte的文件名，3 byte地址， 1 byte isFile

Dirent 512 byte 分为16 * Unit, 共2048个，占1MB

INode 64 byte, 共16 * 1024个，占1MB
