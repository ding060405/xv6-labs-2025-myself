# filesystem_degsin

## large file

### bmap function
    根据传入的文件的inode和logical block number
    
    查找对应地址实际block number

    若未为logical block number分配实际的block，则为其分配

    返回实际block number

### idea

    采用double-indirectly，将原本12个addr(11个direct，1个
    indirect)修改为10个direct，1个indirect，1个double-indirect，
    其中double-indirect中映射512个indirect



### needed struct
```cpp
        // 修改直接映射磁盘块的数量
        #define NDIRECT 10
        // 修改dinode的block映射addr
        struct dinode{
            ...
            int addr[NDIRECT+2];
            ...
        }
        // 修改inode的block映射addr
        struct  inode{
            ...
            int addr[NDIRECT+2];
            ...
        }

        // 函数原型
        static uint
        bmap(struct inode *ip, uint bn)

        //ip为查找文件的inode
        // bn为文件内部的logical block number
```
### processor
#### bmap

- find in direct addr firstly
    bn<NDIRECT

        是否映射实际block
        是 return addr
        否 调用balloc()分配block记录并返回              
    
    bn>NDIRECT
        |
        |
        v
- find in indirect addr
    判断是否有indirect addr 无 分配block
        |
        v
    bread() 读取block
    获取data
        |
        v
    判断data中逻辑块号bn是否有实际block 无 分配block 调用log_write()修改block
        |
        v
    返回实际block number
    调用release()释放block buffer
- find in double-indirect addr
    判断是否为double-indirect addr分配实际block 无则分配，调用log_write()修改block
        |
        v
    bread() 读取block
        |
        v
    读取block中的data
        |
        v
    判断logical block number位于第几个映射块
    n=bn/NINDIRECT
    判断位于映射块中的第几位
    i=bn%NINDIRECT
        |
        v
    find logical block bn
    无则分配并写回
        |
        v
    release() 释放缓存

#### itrunc

- direct addr
        bfree() 直接释放并置0
- indirect addr
        using bread() to get block 
        然后释放block中每个映射的block
        最后释放indirect block
        释放缓存
- double indirect addr
        读取double-indirect block
        读取下一级block
        释放二级block中每个被映射的block
        释放二级block
        释放缓存
        释放double-indirect block
        释放缓存
- ip->size=0

## symbolic links

### to do
    实现system call of symbolic link
    fail: return -1;
    success: return 0;
### idea
    add flag O_NOFOLLOW  不去解析symbolic link
    write path into target block
    add judge into sys_opne when opening file wtih T_SYMLINK and without O_NOFOLLOW
    limit times of loop to avoid a cycle,about 10 times 
### needed structure
```cpp
    void symlink(char *target, char *path)
    #define T_SYMLINK
    #define O_NOFOLLOW
```
### processor
#### sys_symlink
    get arguments into target and path
        |
        v
    create inode of target
        |
        v
    write path into block of target
#### sys_open
    judge if the file with T_SYMLINK and without O_NOFOLLOW
    |
    v
    if yes
    |
    v
    looply read file symlink until symlink end or 10 times
    |
    v
    judge wether loop times is beyond 10 times
    if loop times is beyond 10 times return -1
    |
    v
    continue sys_open
### extra modify
- add system call number in kernel/syscall.h
- add entry of symlink in user/usys.pl and user/user.h
- modify Makefile
- Add a new file type (T_SYMLINK) to kernel/stat.h
- Add a new flag to kernel/fcntl.h, (O_NOFOLLOW)
- modify syscall.c
