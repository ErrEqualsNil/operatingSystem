Operating System Course Design

1. Inode（16B） 64inode/block -> 1MB
   
inode内储存内容：
  1. 文件字节数(42kb => )
  2. 时间戳（创建，修改，打开）
  3. 链接数（硬链接）
  4. 10个直属block地址
  5. 1个间接block地址（对应32个地址（1kb / 3b 向2的n次方取整））

2. Dirent 1MB 512B（）
  6. 32B 单位，包括文件对应inode地址/dirent地址（3B） + isFile 1B + 文件名28B
  7. 共16个单位

3. 空间划分
共16MB， 1KB / block， 共16 * 1024个block， 1MB inode, 1MB dir, 14MB 储存空间
地址为24bits长度 = 3B 14位表示哪个block, 10位表示block中哪个位置

4. 存表
Inode 空闲表
Dirent 空闲表
Block 空闲表

5. 永久化存储
