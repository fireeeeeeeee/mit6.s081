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

#define bcacheNum 13
struct buf bufs[NBUF];
struct {
  struct spinlock lock;
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bcache[bcacheNum];

void
binit(void)
{
  struct buf *b;

  for(int i=0;i<bcacheNum;i++)
  {
  	initlock(&bcache[i].lock, "bcache");
	bcache[i].head.prev = &bcache[i].head;
	bcache[i].head.next = &bcache[i].head;
  }
  
  for(b = bufs; b < bufs+NBUF; b++){
    b->next = bcache[0].head.next;
    b->prev = &bcache[0].head;
    b->bid=0;
    b->ticks=0;
    initsleeplock(&b->lock, "buffer");
    bcache[0].head.next->prev = b;
    bcache[0].head.next = b;
  }
}

uint hash(uint a1,uint a2)
{
//	return 0;
	return (a1+1)*(a2+1)%bcacheNum;
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int hid=hash(dev,blockno);
  acquire(&bcache[hid].lock);

  // Is the block already cached?
  for(b = bcache[hid].head.next; b != &bcache[hid].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache[hid].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcache[hid].lock);
  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  
  int chosi=-1;
  for(int i=0;i<bcacheNum;i++)
  {
    acquire(&bcache[i].lock);
    if(i==hid)
    {
    	//double check for dont map twice
      
	  for(struct buf *b = bcache[hid].head.next; b != &bcache[hid].head; b = b->next){

		if(b->dev == dev && b->blockno == blockno ){

		 if(chosi!=-1)
		 {
		 	release(&bcache[chosi].lock);
		 }
		 
		  b->refcnt++;
		  release(&bcache[hid].lock);
		  acquiresleep(&b->lock);
		  return b;
		}
	  }
    }
  	for(struct buf *t = bcache[i].head.next;t!=&bcache[i].head;t=t->next)
  	{
  	  if(t->refcnt==0)
  	  {
  	  	if(chosi==-1)
  	  	{
  	  		chosi=i;
  	  		b=t;
  	  	}
  	  	else if(t->ticks<b->ticks)
  	  	{
			if(chosi!=hid&&chosi!=i)
			{
				release(&bcache[chosi].lock); 	  	 		  			
			}
  	  		chosi=i;
  	  		b=t;

  	  	}
  	  }
  	}
    if(i!=chosi&&i!=hid)
  	release(&bcache[i].lock);
  }
  
  if(chosi==-1)
  {
  panic("bget: no buffers");
  }
  if(chosi!=hid)
  {
    b->next->prev = b->prev;
    b->prev->next = b->next;
  	
  	
    b->next = bcache[hid].head.next;
    b->prev = &bcache[hid].head;
    bcache[hid].head.next->prev = b;
    bcache[hid].head.next = b;
    
  	b->bid=hid;
  	
  	release(&bcache[chosi].lock);
  }

  b->dev = dev;
  b->blockno = blockno;
  b->valid = 0;
  b->refcnt = 1;
  release(&bcache[hid].lock);
  acquiresleep(&b->lock);
  
  return b; 
 
  
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
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
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int hid;
  while(1)
  {
  	hid=b->bid;
  	acquire(&bcache[hid].lock);
  	if(hid==b->bid)	break;		
  	release(&bcache[hid].lock);
  }
  
  b->refcnt--;
  if(b->refcnt<0)
  {
   panic("brelse");
  }
  if (b->refcnt == 0) {
  	b->ticks=getTick();
  }
  
  release(&bcache[hid].lock);
}

void
bpin(struct buf *b) {
  int hid;
  while(1)
  {
  	hid=b->bid;
  	acquire(&bcache[hid].lock);
  	if(hid==b->bid)	break;		
  	release(&bcache[hid].lock);
  }
  b->refcnt++;
  release(&bcache[b->bid].lock);
}

void
bunpin(struct buf *b) {
  int hid;
  while(1)
  {
  	hid=b->bid;
  	acquire(&bcache[hid].lock);
  	if(hid==b->bid)	break;		
  	release(&bcache[hid].lock);
  }
  b->refcnt--;
  
  release(&bcache[b->bid].lock);
}


