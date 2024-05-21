#ifndef OSMM_H
#define OSMM_H

#define MM_PAGING
#define PAGING_MAX_MMSWP 4 /* max number of supported swapped space */
#define PAGING_MAX_SYMTBL_SZ 30
#include <semaphore.h>

// typedef unsigned int uint32_t;
typedef char BYTE;
typedef uint32_t addr_t;

struct pgn_t{
   int pgn;
   struct pgn_t *pg_next; 
};

/*
 *  Memory region struct
 */
struct vm_rg_struct {
   unsigned long rg_start;
   unsigned long rg_end;

   struct vm_rg_struct *rg_next;
};

/*
 *  Memory area struct
 */
struct vm_area_struct {
   unsigned long vm_id;
   unsigned long vm_start;
   unsigned long vm_end;

   unsigned long sbrk;
/*
 * Derived field
 * unsigned long vm_limit = vm_end - vm_start
 */
   struct mm_struct *vm_mm;
   struct vm_rg_struct *vm_freerg_list;
   struct vm_area_struct *vm_next;
};

/* 
 * Memory management struct
 */
struct mm_struct {
   uint32_t *pgd;

   struct vm_area_struct *mmap;

   /* Currently we support a fixed number of symbol */
   struct vm_rg_struct symrgtbl[PAGING_MAX_SYMTBL_SZ];

   /* list of free page */
   struct pgn_t *fifo_pgn;
   sem_t memlock;
};

/*
 * FRAME/MEM PHY struct
 */
struct framephy_struct { 
   int fpn;
   struct framephy_struct *fp_next;

   /* Resereed for tracking allocated framed */
   struct mm_struct* owner;
};
struct TLB_node
{
   uint32_t MEMPHY; // respresent physical mermory
   uint32_t MEMVIR; // respresent virtual mermory
   uint32_t pid;    // process ID.
   int id;
   uint32_t indexOfRegister;
   int isWrite; // is TLB cache written? 0:1
   struct TLB_node *next;
   struct TLB_node *prev;
};

struct TLB_cache
{
   int capacity; // number of nodes TLB can hold.
   int num;      // number of nodes TLB has now.
   struct TLB_node *head;
   struct TLB_node *tail;
   struct TLB_node *freehead;
   struct TLB_node *freetail;
};
struct memphy_struct {
   /* Basic field of data and size */
   BYTE *storage;
   int maxsz;
   sem_t MEMPHY_lock;
   
   /* Sequential device fields */ 
   int rdmflg;
   int cursor;

   /* Management structure */
   struct framephy_struct *free_fp_list;
   struct framephy_struct *used_fp_list;
   struct TLB_node *tlb_head;
   struct TLB_node *tlb_tail;
};

#endif
