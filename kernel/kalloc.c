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


struct kmem kmems[NCPU];
void
kinit()
{
  for(int i=0; i<NCPU; i++){
    initlock(&kmems[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(int count=0; p + PGSIZE <= (char*)pa_end; p += PGSIZE, count++) {
    if(((uint64)p % PGSIZE) != 0 || (char*)p < end || (uint64)p >= PHYSTOP)
      panic("freerange");

    memset(p, 1, PGSIZE);
    struct run *r = (struct run*)p;
    count = count % NCPU;
    acquire(&kmems[count].lock);
    r->next = kmems[count].freelist;
    kmems[count].freelist = r;
    release(&kmems[count].lock);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int id;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
//////////////////////////////////
  push_off();
  id = cpuid();
  acquire(&kmems[id].lock);
  r->next = kmems[id].freelist;
  kmems[id].freelist = r;
  release(&kmems[id].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r = 0;
  int id;
  int count;
////////////////////////////////////////
  push_off();
  id = cpuid();

  //窃取内存块
  count = id;
  while(!r) {
    if(kmems[count].freelist) {
      
      acquire(&kmems[count].lock);
      r = kmems[count].freelist;
      if(r)
        kmems[count].freelist = r->next;
      release(&kmems[count].lock);
    }
    
    count = (count+1) % NCPU;
    if(count == id) break;  //所有CPU均无空闲页
  }
  pop_off();

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    return (void*)r;
  } else
    return (void*)0;  
}
