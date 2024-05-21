/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access 
 * and runs at high speed
 */


#include "mm.h"
#include <stdlib.h>
#include<stdio.h>
#define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
 int tlb_cache_read(struct TLB_cache *tlb, uint32_t pid, uint32_t virAdd, uint32_t *temp) {
    if (tlb->head == NULL) {
        printf("Error!\n");
        return -1;
    }

    struct TLB_node *current_node = tlb->head;
    while (current_node != NULL) {
        if (current_node->MEMVIR == virAdd && current_node->isWrite == 1 && current_node->pid == pid) {
            *temp = current_node->MEMPHY;

            if (current_node != tlb->head) {
                if (current_node == tlb->tail) {
                    tlb->tail = current_node->prev;
                    tlb->tail->next = NULL;
                } 
                else {current_node->next->prev = current_node->prev;}
                current_node->prev->next = current_node->next;
                current_node->prev = NULL;
                current_node->next = tlb->head;
                tlb->head->prev = current_node;
                tlb->head = current_node;
            }

            return 0;
        }
        current_node = current_node->next;
    }

    return -1;
}

int tlb_cache_write(struct TLB_cache *tlb, uint32_t pid, uint32_t MEMPHY, uint32_t MEMVIR) {
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
    if (tlb->head == NULL) {
        printf("Error\n");
        return -1;
    }
    struct TLB_node *current_node = tlb->head;
    while (current_node != NULL) {
        if (current_node->MEMVIR == MEMVIR && current_node->isWrite == 1 && pid == current_node->pid) {
            current_node->MEMPHY = MEMPHY;
            current_node->MEMVIR = MEMVIR;
            current_node->pid = pid;
            current_node->isWrite = 1;

            if (current_node != tlb->head) {
                if (current_node == tlb->tail) {
                    current_node->prev->next = NULL;
                    tlb->tail = current_node->prev;
                    current_node->prev = NULL;
                } 
                else {
                    current_node->prev->next = current_node->next;
                    current_node->next->prev = current_node->prev;
                    current_node->next = current_node->prev = NULL;
                     }
                // move currentnode to head
                current_node->next = tlb->head;
                tlb->head->prev = current_node;
                tlb->head = current_node;
            }
            return 0;
        }
        current_node = current_node->next;
    }
    struct TLB_node *checknode = tlb->head;
    while (checknode != NULL) {
    if (checknode->MEMVIR == MEMVIR && checknode->isWrite == 1 && checknode->pid == pid) {
        checknode->MEMPHY = MEMPHY;
        checknode->MEMVIR = MEMVIR;
        checknode->pid = pid;
        checknode->isWrite = 1;  // Assumed you intended this to be set

        if (checknode != tlb->head) {
            if (checknode == tlb->tail) {
                checknode->prev->next = NULL;
                tlb->tail = checknode->prev;
                checknode->prev = NULL;
            } 
            else {
                checknode->prev->next = checknode->next;
                checknode->next->prev = checknode->prev;
                checknode->next = checknode->prev = NULL;
            }
            // move checknode to head
            checknode->next = tlb->head;
            tlb->head->prev = checknode;
            tlb->head = checknode;
        }
        return 0;
    }
    checknode = checknode->next;
    }
    


    struct TLB_node *node = tlb->tail;
    node->MEMPHY = MEMPHY;
    node->MEMVIR = MEMVIR;
    node->pid = pid;
    node->isWrite = 1;

    tlb->tail = tlb->tail->prev;
    tlb->tail->next = NULL;
    node->next = node->prev = NULL;

    node->next = tlb->head;
    tlb->head->prev = node;
    tlb->head = node;
    return -1;
}



/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int TLBMEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}


/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = data;

   return 0;
}

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 */


int TLBMEMPHY_dump(struct memphy_struct * mp)
{
   /*TODO dump memphy contnt mp->storage 
    *     for tracing the memory content
    */

   return 0;
}


/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct TLB_cache *tlb, int max_size)
{
    tlb->head = tlb->tail = NULL;
    tlb->capacity = max_size;
    tlb->num = 0;
    tlb->freehead = tlb->freetail = NULL;
    for (int i = 0; i < max_size; i++) {
        struct TLB_node *node = malloc(sizeof(struct TLB_node));
        node->next = node->prev = NULL;
        if (tlb->freehead == NULL) {
            tlb->freehead = tlb->freetail = node;
            tlb->num++;
        } 
        else {
            tlb->freetail->next = node;
            node->prev = tlb->freetail;
            tlb->freetail = node;
            tlb->num++;
            }
        node->id = i;
    }
    return 0;
}

// #endif

