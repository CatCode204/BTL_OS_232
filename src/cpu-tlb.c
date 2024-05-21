/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

int tlb_change_all_page_tables_of(struct pcb_t *proc,  struct memphy_struct * mp)
{
  /* TODO update all page table directory info 
   *      in flush or wipe TLB (if needed)
   */

  return 0;
}

int tlb_flush_tlb_of(struct TLB_cache *flush)
{
  /* TODO flush tlb cached*/
  struct TLB_node *ptr, *tmp;
  ptr = flush->head;
  if (ptr != NULL) {
      while (ptr != NULL) {
          if (ptr->isWrite == 1) {
              printf("memphy: %5d | memv: '%5d'| pid: '%d'\n", ptr->MEMPHY, ptr->MEMVIR, ptr->pid);
          }
          tmp = ptr;
          ptr = ptr->next;
          free(tmp);
      }
      flush->head = NULL;
      flush->tail = NULL;
  } else {
      printf("TLB Cache is empty.\n");
  }
  ptr = flush->freehead;
  if (ptr != NULL) {
      while (ptr != NULL) {
          tmp = ptr;
          ptr = ptr->next;
          free(tmp);
      }
      flush->freehead = NULL;
      flush->freetail = NULL;
  } else {
      printf("Freelist is empty.\n");
  }
  return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr, val;

  struct TLB_cache *tlb = proc->tlb;  // proc is added TLB_cache struct.
		
  if (size > proc->tlb->num){
    printf("missing mermory region! \n");
    return -1;        
  }

  for (int i=0;i<size;i++){
    
    struct TLB_node *freeTLB = tlb->freetail;
    proc->tlb->freetail = proc->tlb->freetail->prev;
    proc->tlb->freetail->next = NULL;


    freeTLB->next = freeTLB->prev = NULL;
    freeTLB->indexOfRegister = reg_index;
    freeTLB->isWrite = 0;       
    if (proc->tlb->head == NULL){
          proc->tlb->head = proc->tlb->tail = freeTLB;
          proc->tlb->num--; 
    }
    else {
      proc->tlb->tail->next = freeTLB;
      freeTLB->prev = proc->tlb->tail;
      proc->tlb->tail = freeTLB;
      proc->tlb->num--;
    }
  }
  /* By default using vmaid = 0 */
  val = __alloc(proc, 0, reg_index, size, &addr);

  /* TODO update TLB CACHED frame num of the new allocated page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  return val;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  struct TLB_node *temp = proc->tlb->head;
  if(temp == NULL){
	printf("Invalid!\n");
	return -1;
  }
  while(temp != NULL){
    if(temp->indexOfRegister == reg_index){
      struct TLB_node *tlbnode = temp;
      if(proc->tlb->head == temp) {
        if(proc->tlb->head == proc->tlb->tail){
          proc->tlb->head = proc->tlb->tail = NULL;
        }
        else{
          proc->tlb->head = proc->tlb->head->next;
          proc->tlb->head->prev = NULL;
          }
        }
        else if (temp == proc->tlb->tail){
          proc->tlb->tail = proc->tlb->tail->prev;
          proc->tlb->tail->next = NULL;
        }
        else {
          tlbnode->next->prev = tlbnode->prev;
          tlbnode->prev->next = tlbnode->next;
        }
        temp = temp->next;
        tlbnode->isWrite = 0;

        //giai phong con tro hien tai
        tlbnode->next = tlbnode->prev = NULL;
        //Them vung nho duoc gai phong vao freelist
        proc->tlb->freetail->next = tlbnode;
        tlbnode->prev = proc->tlb->freetail;
        proc->tlb->freetail = tlbnode;

      proc->tlb->num++;
      continue;
    }
    temp = temp->next;
  }
  __free(proc, 0, reg_index);

  /* TODO update TLB CACHED frame num of freed page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  return 0;
}


/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t destination) 
{
  BYTE data;
  uint32_t frmnum = -1;
  int srcaddr = source + offset;
	
  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/
	
#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at read region=%d offset=%d\n", 
	         source, offset);
  else 
    printf("TLB miss at read region=%d offset=%d\n", 
	         source, offset);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  int val = __read(proc, 0, source, offset, &data);

  destination = (uint32_t) data;
  tlb_cache_write (proc->tlb, proc->pid, destination, srcaddr);
  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  return val;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val;
  uint32_t frmnum = -1;

  /* TODO retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/

#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
	          destination, offset, data);
	else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
            destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  val = __write(proc, 0, destination, offset, data);
    tlb_cache_write (proc->tlb, proc->pid, destination, destination + offset);
  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  return val;
}

//#endif
