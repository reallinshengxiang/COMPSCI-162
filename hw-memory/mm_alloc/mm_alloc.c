/*
 * mm_alloc.c
 */

#include "mm_alloc.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef struct block {
    size_t size;
    struct block *next,*prev;
    int free;
    int data[];
} block_m;

block_m* list_head = NULL;

void* mm_malloc(size_t size) {
  //TODO: Implement malloc
  if (size == 0)  return NULL;
  block_m* new_block = sbrk(size + sizeof(block_m));
  if (new_block == (void*)-1)  return NULL;
  memset(new_block, 0, size + sizeof(block_m));
  new_block->size = size;
  new_block->free = 1;
  if (list_head == NULL) 
  {
    list_head = new_block;
    new_block->free = 0;
    return new_block->data;
  }
  block_m *current_block = list_head, *prev = NULL;
  while (current_block) {
      if (current_block->free && current_block->size >= size) {
        if(current_block->size <= size + sizeof(block_m)){
         current_block->free = 0;
         return current_block->data;
        }
        else{
          block_m* split_block = (block_m*)((char*)current_block + size);
          split_block->size = current_block->size - size - sizeof(block_m);
          split_block->free = 1;
          split_block->next = current_block->next;
          current_block->size = size;
          current_block->next = split_block;
          current_block->free = 0;
          return current_block->data;
        }
      }
      prev = current_block;
      current_block = current_block->next;
  }
  prev->next = new_block;
  new_block->next = NULL;
  new_block->free = 0;
  return new_block->data;
}

void* mm_realloc(void* ptr, size_t size) {
  //TODO: Implement realloc
  if (ptr == NULL) return mm_malloc(size);
  if (size == 0) {
      mm_free(ptr);
      return NULL;
  }
  block_m *current_block = (block_m *)(ptr - (sizeof(size_t) + 2 * sizeof(block_m*) + sizeof(int)));
  if (current_block->size >= size)   return ptr;
  void* new_ptr = mm_malloc(size);
  if (new_ptr == NULL)  return NULL;
  memcpy(new_ptr, ptr, current_block->size);
  mm_free(ptr);
  return new_ptr;
}

void mm_free(void* ptr) {
  //TODO: Implement free
  if (ptr == NULL)   return;
  block_m *current_block = (block_m *)(ptr - (sizeof(size_t) + 2 * sizeof(block_m*) + sizeof(int)));
  current_block->free = 1;
  memset(current_block->data, 0, current_block->size);
  while (current_block->next && current_block->next->free) {
    current_block->size = current_block->size + current_block->next->size;
    current_block->next = current_block->next->next;
  }
  current_block = list_head;
  while(current_block){
    if(current_block->free && current_block->next!=NULL) {
      if(current_block->next->free) {
          current_block->size = current_block->size + current_block->next->size;
          current_block->next = current_block->next->next;
      }
    }
    current_block = current_block->next;
  }
  current_block = list_head;
  while(current_block){
    if(current_block->free) memset(current_block->data, 0, current_block->size);
    current_block = current_block->next;
  }
  
}