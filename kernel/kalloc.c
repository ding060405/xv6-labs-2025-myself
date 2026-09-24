// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;
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

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)

    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  acquire(&refcnt.lock);
  int t=refcnt.cnt[((uint64)pa-KERNBASE)/PGSIZE];  
  if(t>0) {
    refcnt.cnt[((uint64)pa-KERNBASE)/PGSIZE]--;
    t--;
  }
  release(&refcnt.lock);
  // Fill with junk to catch dangling refs.
  if(t==0) {
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  //increasecnt((uint64)((void*)r));
  if(r) setref((uint64)((void*)r));
  return (void*)r;
}
