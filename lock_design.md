# lock_design
## memory allocator
### background
    多个cpu共用同一个memory lock(spinlock),会导致cpu资源持续占用，
    热冲突等问题，为提高cpu的利用率，为每个cpu都设置一个memmory
     lock和freelist
### to do
- add memory lock for every cpu
- add "steal freelist",but hopefully be infrequent
- add own freelist for every cpu
### idea
- 设置mermory lock数组，每个cpu持有对应下标的锁
- 将RAM平分给每个cpu的freelist
- 增加偷取机制，采用相邻优先偷取，减少热冲突,依次偷取多页，避免频繁访问
### struct
```cpp
struct kmem{
  struct spinlock lock;
  struct run *freelist;
};
struct kmem km[NCPU];
```
### modify part
```cpp
void kalloc();
void kfree(void *pa);
void freerange(void *pa_start, void *pa_end) 
```
### processor
#### freerange
- 为每个cpu分配freelist
    每次将一个page分配给一个cpu，循环分配
#### kalloc
get cpu id
|
v
acquire lock
|
v
freelist is empty?---->yes alloc directly
|
v
steal some freelists(about 8 page) of other cpu near it
|
v
release lock
#### kfree
get cpu id
|
v
acquire lock
|
v
free page
|
v
insert page into its own freelist
|
v
release lock
### note
#### 相邻偷取，避免多个cpu同时访问一个cpu，造成热冲突
#### 一次偷取多页，但不偷取过多，避免多次频繁访问，同时方式持有锁的时间过长
## read-write lock
### background
多个reader同时访问一块内存，可同时访问，writer对内存的修改，一次只允许一个writer，且writer修改内存时不允许reader访问
### to do
- 解决记录reader数目问题
- 解决在有readers访问时，writer请求访问
- 解决多个writer请求访问
### idea
- 增加readers计数
- 当readers=0时，writer可以访问
- 增加writer等待队列 
### needed structure
```cpp
struct rwspinlock {
  // Replace this with your implementation.
  //struct spinlock l;
  int writer; //writer number only 0 or 1
  int readers;// reader number
  char name[5];// lock name
  int waiting_writer;// waiting queue
  struct cpu *cpu;// own cpu
};
```
### processor
#### reader_acquire
loop atomicly get writer
|
v
get 0 to next step
else loop
|
v
loop atomicly get waiting writer
|
v
get 0 to next step
else loop
|
v
atomicly add readers
|
v
judge wether writer and waiting_writer is 0
|
v
if they are all 0 break loop
else sub reader atomicly and continue loop
#### reader_release
atomicly get reader number
|
v
if reader isn't 0 panic()
|
v
atomicly sub reader
#### writer_acquire
add waiting_writer atomicly
|
v
get writer number
|
v
if writer=0 break
else continue loop
|
v
sub waiting_writer
|
v
set cpu
#### writer_release
get writer number
|
v
if writer=0 panic()
|
v
sub writer
|
v
release cpu
## note
- 需要同时考虑是否有writer或者waiting_writer
- 修改reader后，需要再次判断是否没有writer或者waiting_writer
- reader可以多个并行访问，不需要设置cpu
- writer优先加入等待队列，无writer时，再从等待队列进入

