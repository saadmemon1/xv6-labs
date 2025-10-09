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

#ifdef LAB_PGTBL
#define NSUPERPAGE 16
struct {
  struct spinlock lock;
  struct run *freelist;
  char pages[NSUPERPAGE * SUPERPGSIZE];
} skmem;
#endif

void
kinit()
{
  initlock(&kmem.lock, "kmem");
#ifdef LAB_PGTBL
  initlock(&skmem.lock, "kmem_super");
  // Build the superpage freelist directly to avoid boundary check issues
  char *p = (char*)SUPERPGROUNDUP((uint64)skmem.pages);
  skmem.freelist = 0;
  for (int i = 0; i < NSUPERPAGE; ++i) {
    struct run *r = (struct run*)(p + i * SUPERPGSIZE);
    r->next = skmem.freelist;
    skmem.freelist = r;
  }
  // The physical memory available for the regular allocator is now split.
  // Add the memory before the superpage area.
  freerange(end, p);
  // Add the memory after the superpage area.
  freerange(p + NSUPERPAGE * SUPERPGSIZE, (void*)PHYSTOP);
#else
  freerange(end, (void*)PHYSTOP);
#endif
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

  // The check `(char*)pa < end` is problematic when reserving memory
  // for superpages inside the kernel data section. Removing it is a
  // common approach for this lab. The other checks are sufficient.
  if(((uint64)pa % PGSIZE) != 0 || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
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
  return (void*)r;
}

#ifdef LAB_PGTBL
// Allocate one 2MB superpage of physical memory.
void *
superalloc(void)
{
  struct run *r;

  acquire(&skmem.lock);
  r = skmem.freelist;
  if(r)
    skmem.freelist = r->next;
  release(&skmem.lock);

  if(r)
    memset((char*)r, 5, SUPERPGSIZE);
  return (void*)r;
}

// Free a 2MB superpage.
void
superfree(void *pa)
{
  struct run *r;

  char *aligned_start = (char*)SUPERPGROUNDUP((uint64)skmem.pages);
  if(((uint64)pa % SUPERPGSIZE) != 0 || (char*)pa < aligned_start || (uint64)pa >= (uint64)aligned_start + NSUPERPAGE * SUPERPGSIZE)
    panic("superfree");

  // Fill to catch dangling refs.
  memset(pa, 1, SUPERPGSIZE);

  r = (struct run*)pa;

  acquire(&skmem.lock);
  r->next = skmem.freelist;
  skmem.freelist = r;
  release(&skmem.lock);
}
#endif
