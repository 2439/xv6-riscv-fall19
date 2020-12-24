// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKETS 13

struct {
  struct spinlock lock[NBUCKETS];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf hashbucket[NBUCKETS];
} bcache;

void
binit(void)
{
  struct buf *b;
  int count=0;

  for(int i=0; i<NBUCKETS; i++){
    initlock(&bcache.lock[i], "bcache");
    bcache.hashbucket[i].prev = &bcache.hashbucket[i];    // 将缓存块分为多个哈希桶
    bcache.hashbucket[i].next = &bcache.hashbucket[i];
  }

  // Create linked list of buffers
  for(b = bcache.buf; b < bcache.buf+NBUF; b++, count++){   // 为每个buf添加链接
    count = count % NBUCKETS;   // 第count桶
    b->next = bcache.hashbucket[count].next;    // 加在头部
    b->prev = &bcache.hashbucket[count];
    initsleeplock(&b->lock, "buffer");
    bcache.hashbucket[count].next->prev = b;
    bcache.hashbucket[count].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int id = blockno % NBUCKETS;    // id：block对应的哈希桶id
  int count = id;

  acquire(&bcache.lock[id]);   //访问哈希桶的锁
  // Is the block already cached?是否缓存
  for(b = bcache.hashbucket[id].next; b != &bcache.hashbucket[id]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[id]);
      acquiresleep(&b->lock);    // 块的锁
      return b;
    }
  }
  release(&bcache.lock[id]);

  // Not cached; recycle an unused buffer.  无缓存
  while(1) {
    acquire(&bcache.lock[count]);
    for(b = bcache.hashbucket[count].prev; b != &bcache.hashbucket[count]; b = b->prev){
      if(b->refcnt == 0) {   // 无引用，可替换
        b->dev = dev;     // 页面重新赋值
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        
        b->next->prev = b->prev;    // 将当前页面换下
        b->prev->next = b->next;
        release(&bcache.lock[count]);

        acquire(&bcache.lock[id]);
        b->next = bcache.hashbucket[id].next;   // 插入该页面到该桶
        b->prev = &bcache.hashbucket[id];
        bcache.hashbucket[id].next->prev = b;
        bcache.hashbucket[id].next = b;
        release(&bcache.lock[id]);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.lock[count]);   // count桶内没有可换下的缓存空间
    count = (count+1) % NBUCKETS;
    if(count == id) {   // 都没有缓存空间
      break;
    }
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int count = b->blockno % NBUCKETS;
  acquire(&bcache.lock[count]);
  b->refcnt--;      // 当前count引用减少
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;    // 插入缓存区
    b->prev->next = b->next;
    b->next = bcache.hashbucket[count].next;
    b->prev = &bcache.hashbucket[count];
    bcache.hashbucket[count].next->prev = b;
    bcache.hashbucket[count].next = b;
  }
  
  release(&bcache.lock[count]);
}

void
bpin(struct buf *b) {
  int count = b->blockno % NBUCKETS;
  acquire(&bcache.lock[count]);
  b->refcnt++;
  release(&bcache.lock[count]);
}

void
bunpin(struct buf *b) {
  int count = b->blockno % NBUCKETS;
  acquire(&bcache.lock[count]);
  b->refcnt--;
  release(&bcache.lock[count]);
}