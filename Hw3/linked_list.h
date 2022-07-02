
#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "util.h"


typedef struct LinkedList LinkedList_t;

LinkedList_t* LinkedList_create();

int LinkedList_is_empty(const LinkedList_t* const list);

int64_t LinkedList_size(const LinkedList_t* const list);

int LinkedList_add(LinkedList_t* list, void* data);

void* LinkedList_get(const LinkedList_t* const list, int index);

void LinkedList_remove(LinkedList_t* list, const void* const data);

void* LinkedList_remove_by_idx(LinkedList_t* list, int index);

void LinkedList_clear_and_free(LinkedList_t* list, void (*free_item)(void* p_item));



char* LinkedList_stringify(const void* const p_list, char* (*item_stringify)(const void* const item));

size_t LinkedList_hash(const void* const p_list, size_t (*item_hash)(const void* const item));

int LinkedList_compare(
    const void* const p_list1, const void* const p_list2,
    int (*item_compare)(const void* const item1, const void* const item2)
);

#endif
