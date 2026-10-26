# cow_design

## to do
    implement copy-and-write fork
## idea
    when copying the physical memory,the operate
    system doesn't allocate a physical memory page until
    it is needed
    It create a PTE refering previous physical memory
- add a variable to count how many process use it
- add a flag(COW) to indentify it is copy-and-write
- if the copies are needed,the kernel will invoke vmfault to allcate physical memory for it
- only the page with PTE_W need to add COW
- must note the usage of lock
## kernel invoke process
copy
|
v
allocate
|
v
add COW flag
|
v
when the copies are needed
|
v
if is write
|
v
vmfault
## modify
```cpp
int uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
uint64 uvmfault(pagetable_t pagetable, uint64 va, int read)
void kfree(uint64 pa)
uint64 kalloc() 
```
## struct
```cpp
#define PTE_COW (1L<<8) // PTE中，第8，9位位用户自行设置
struct refcnt{
  struct spinlock lock;
  int cnt[(PHYSTOP-KERNBASE)/PGSIZE];
}refcnt;
void
increasecnt(uint64 pa){
  acquire(&refcnt.lock);
  refcnt.cnt[(pa-KERNBASE)/PGSIZE]++;
  release(&refcnt.lock);
  return;
}
void
decreasecnt(uint64 pa){
  acquire(&refcnt.lock);
  refcnt.cnt[(pa-KERNBASE)/PGSIZE]--;
  release(&refcnt.lock);
  return;
}
int getcnt(uint64 pa){
  int n;
  acquire(&refcnt.lock);
  n=refcnt.cnt[(pa-KERNBASE)/PGSIZE];
  release(&refcnt.lock);
  return n;
}
void setref(uint64 pa){
  acquire(&refcnt.lock);
  refcnt.cnt[(pa-KERNBASE)/PGSIZE]=1;
  release(&refcnt.lock);
}
void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&refcnt.lock,"refcnt");
  memset(refcnt.cnt,0,sizeof(refcnt.cnt));
  freerange(end, (void*)PHYSTOP);
}
```
## process
### vmcopy
get old pte
|
v

get the physical address by pte
|
v
if with PTE_W
|
v
add COW
|
v
map new pagetable to old pa
|
v
increase physical memory reference
### vmfault
if with PTE_COW
|
v
decrease memory reference
|
v
allocate new physical memory for the copy
|
v
if only one reference,remove COW
### kfree
if reference is 1
|
v
free memory
|
else 
|
reference decrease 1
### kalloc
if alloc
|
v
set reference to 1

