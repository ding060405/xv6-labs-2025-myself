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

struct kmem{
  struct spinlock lock;
  struct run *freelist;
};
struct kmem km[NCPU];
void
kinit()
{
  //initlock(&kmem.lock, "kmem");
  //char buf[5];
  for(int i=0;i<NCPU;i++){
    initlock(&km[i].lock,"kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  int id=0;
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    //kfree(p);
    struct run *r=(struct run*)p;
    memset(p,1,PGSIZE);
    acquire(&km[id].lock);
    r->next=km[id].freelist;
    km[id].freelist=r;
    release(&km[id].lock);
    id=(id+1)%NCPU;
  }
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

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  push_off();
  int id=cpuid();
  acquire(&km[id].lock);

  //acquire(&kmem.lock);
  //r->next = kmem.freelist;
  //kmem.freelist = r;
  //release(&kmem.lock);
  r->next=km[id].freelist;
  km[id].freelist=r;
  release(&km[id].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();
  int id=cpuid();
  acquire(&km[id].lock);
  //acquire(&kmem.lock);
  r = km[id].freelist;
  if(r){
    km[id].freelist = r->next;
  //release(&kmem.lock);
  //release(&km[id].lock);
  }
  release(&km[id].lock);
  if(r==0){
    struct run *temp_r;
    struct run *tail;
    for(int i=1;i<NCPU;i++){
      int tid=(id+i)%NCPU;
      //if(i==id) continue;
      acquire(&km[tid].lock);
      if(km[tid].freelist){
        temp_r=km[tid].freelist;
        tail=temp_r;
        int cnt=0;
        while(tail->next&&cnt<8){
          tail=tail->next;
          cnt++;
        }
        km[tid].freelist=tail->next;
        tail->next=0;
        r=temp_r;  
        release(&km[tid].lock);
        break;
      }
      release(&km[tid].lock);
    }
    acquire(&km[id].lock);
    //r=km[id].freelist;
    if(r) km[id].freelist=r->next;
    release(&km[id].lock);
  }
  //release(&km[id].lock);
  pop_off();
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
