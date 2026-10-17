#  mmap design
## 实现功能

mmap映射文件到process的地址
- 懒分配
- 
munmap取消映射

## 函数原型 

```cpp
void *mmap(void *addr, size_t len, int prot, int flags,int fd,off_t offset);
    //addr 映射地址
    ///len 映射比特大小
    //prot  映射地址可执行操作
    //flags 映射类别
    // fd 文件描述符
    // offset 偏移量，映射开始位置
int munmap(void *addr, size_t len);
    //addr 释放地址
    //len 释放长度
```
## 需要结构
```cpp
    // VMA
    // 储存映射文件的信息
    struct VMA{
        uint64 addr;  //起始地址
        uint64 len;  //映射长度
        int perm;    //条件码
        int used;   //是否被使用
        struct file *f //映射文件
        int flags;  //共享标志
        uint64 offset; //文件内偏移量
    }
    struct proc{
        ...
        uint64 addrmapped //当前可映射的VMA地址
        ...
    }
```
```
processes address space分布
heap
 .   
 .
 .
MAPBASE     0x40000
 .
 .
 .
TRAMPFRAME
 .
 .
 .
```
## 流程 
- mmap
    ```
        传入参数
            |
            v
        判断addr  --------> addr==-1 ------> return -1
            |
            v
        addr==0
            |
            v
        找到当前进程未使用的vma(p->vma[i].used==0)
            |
            v
        为当前文件映射地址
        p->vma[i].addr=PGROUNDUP(addrmapped)
        储存文件，判断prot，设置perm
            |
            v
        更新addrmapped
        addrmapped+=PGROUNDUP(len);
            |
            v
        退出循环
    ```
- vmfault lazily allocate
    ```
        判断地址-------> addr>MAXVA ----> return 0
            |
            v
        va>p->sz
            |
            v
        VMA lazily allocate
            |
            v
        查找va所在的vma
        p->vma[i].addr<=va&&va<=p->vma[i].addr+p->vma[i].len
            |
            v
        分配内存 mem=kalloc()
            |
            v
        读取文件内容到内存 readi
            |
            v
        映射虚拟内存
            |
            v
        return mem
    ```
- munmap
    ```
        查找需要取消映射的地址
        (addr>=p->vma[i].addr&&addr+len<=p->vma[i].len+p->vma[i].addr)&&p->used==1
            |
            v
        判断是否需要写回----------------------------------------------------->flags==MAP_SHARED
            |                                                                       |
            v                                                                       v
        flags!=MAP_SHARED                                                       逐页写回
            |                                                                       |
            v                                                                       |
        逐页取消映射<----------------------------------------------------------------                                                                        
            |
            v
        判断取消类型(无中间挖洞)
            |
        -----------------------------------
        |               |                 |  
        v               v                 v
        全部取消        头部取消         尾部取消
        映射            映射            映射
        |               |                 |
        v               v                 v
        减少文件        更新起始         更新映射长度
        引用            地址addr            |
        |               更新映射            |
        v               长度                |
        清除当前         |                  |
        VMA             |                   |
        -------------------------------------
            |
            v
            退出循环       
    ```
## 其他function配合
- exit
    ```
        进程结束
            |
            v
        用户未调用munmap
            |
            v
        exit为了确保页表正常释放
            |
            v
        优先释放所有VMA
            |
            v
        参考munmap释放所有VMA
    ```
- fork
    ```
        进程fork子进程
            |
            v
        子进程应该继承父进程所映射的所有VMA
    ```
  