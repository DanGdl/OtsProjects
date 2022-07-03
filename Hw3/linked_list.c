
#include "linked_list.h"
#include <stdint.h>

#define ITEM_NULL "NULL"
#define TEMPLATE_LINKED_LIST_EMPTY "LinkedList[]"
#define TEMPLATE_LINKED_LIST "LinkedList[ %s ]"



typedef struct ListNode {
    struct ListNode* next;
    struct ListNode* prev;
    void* data;
} ListNode_t;

typedef struct LinkedList {
    ListNode_t* head;
    ListNode_t* last;
    int64_t size;
} LinkedList_t;



LinkedList_t* LinkedList_create() {
    LinkedList_t* l;
    l = malloc(sizeof(*l));
    if (l == NULL) {
        return NULL;
    }
    l -> head = NULL;
    l -> last = NULL;
    l -> size = 0;
    return l;
}

int LinkedList_is_empty(const LinkedList_t* const list) {
    if (list == NULL) {
        return 1;
    }
    return list -> head == NULL;
}

int64_t LinkedList_size(const LinkedList_t* const list){
    if (list == NULL) {
		return 0;
    }
    return list -> size;
}

ListNode_t* LinkedList_create_node(void* data, ListNode_t* prev) {
    ListNode_t* node;
    node = malloc(sizeof(*node));
    if (node == NULL) {
        return NULL;
    }
    node -> next = NULL;
    node -> prev = prev;
    node -> data = data;
    return node;
}

int LinkedList_add(LinkedList_t* list, void* data) {
    if (list == NULL) {
        return -1;
    }
    if (data == NULL) {
        return -1;
    }
    if (list -> head == NULL) {
        ListNode_t* node = LinkedList_create_node(data, NULL);
        if (node == NULL) {
            return -1;
        }

        list -> head = node;
        list -> last = node;
    } else {
        ListNode_t* node = LinkedList_create_node(data, list -> last);
        if (node == NULL) {
            return -1;
        }
        list -> last -> next = node;
        list -> last = node;
    }
    list -> size += 1;
    return 0;
}

void* LinkedList_get(const LinkedList_t* const  list, int index) {
    if (list == NULL || list -> head == NULL) {
        return NULL;
    }
    int i = 0;
    ListNode_t* node = list -> head;
    while (i != index) {
        node = node -> next;
        i++;
    }

    if (node != NULL && i == index) {
        return node -> data;
    }
    return NULL;
}

void LinkedList_remove(LinkedList_t* list, const void* const data) {
    if (list == NULL || list -> head == NULL) {
        return;
    }

    ListNode_t* node = list -> head;
    while (node != NULL && node -> data != data) {
        node = node -> next;
    }

    if (node != NULL && node -> data == data) {
    	if (node -> prev != NULL) {
            node -> prev -> next = node -> next;
    	}
        if (node -> next != NULL) {
            node -> next -> prev = node -> prev;
        }
        if (node == list -> head) {
            list -> head = node -> next;
        }
        if (node == list -> last) {
            list -> last = node -> prev;
        }
        node -> next = NULL;
        node -> prev = NULL;
        node -> data = NULL;
        list -> size -= 1;
        free(node);
    }
}

void* LinkedList_remove_by_idx(LinkedList_t* list, int index) {
    if (list == NULL || list -> head == NULL) {
        return NULL;
    }

    ListNode_t* node = list -> head;
    int idx = 0;
    while (node != NULL && idx != index) {
        node = node -> next;
        idx++;
    }

    void* data = NULL;
    if (node != NULL && idx == index) {
        if (node -> prev != NULL) {
            node -> prev -> next = node -> next;
        }
        if (node -> next != NULL) {
            node -> next -> prev = node -> prev;
        }
        if (node == list -> head) {
            list -> head = node -> next;
        }
        if (node == list -> last) {
            list -> last = node -> prev;
        }

        data = node -> data;

        node -> next = NULL;
        node -> prev = NULL;
        node -> data = NULL;
        list -> size -= 1;
        free(node);
    }
    return data;
}

void LinkedList_clear_and_free(LinkedList_t* list, void (*free_item)(void* p_item)) {
    while (LinkedList_size(list) != 0) {
        void* val = LinkedList_remove_by_idx(list, 0);
        if (val != NULL && free_item != NULL) {
			free_item(val);
        }
    }
    free(list);
}



size_t LinkedList_hash(const void* const p_list, size_t (*item_hash)(const void* const item)) {
    if (p_list == NULL) {
        return 0;
    }
    size_t hash = 0;
    LinkedList_t* list = (LinkedList_t*) p_list;
    ListNode_t* node = list -> head;
    while (node != NULL) {
        size_t node_hash;
        if (item_hash == NULL) {
            node_hash = hash_number(node -> data, sizeof(node -> data));
        } else {
            node_hash = item_hash(node -> data);
        }
        hash = hash_number_iteraction(hash + node_hash);
        node = node -> next;
    }
    return hash_number_end(hash);
}


char* LinkedList_resize_items_str(char* items, const char* const item_str, int extra) {
    if (items == NULL) {
        const int new_length = strlen(item_str) + extra;
        items = calloc(sizeof(char), new_length);
    } else {
        const int new_length = strlen(items) + strlen(item_str) + extra;
        items = realloc(items, sizeof(char) * new_length);
    }
    return items;
}

char* LinkedList_stringify(const void* const p_list, char* (*item_stringify)(const void* const item)) {
    if (p_list == NULL) {
        return NULL;
    }
    char* items = NULL;
    LinkedList_t* list = (LinkedList_t*) p_list;
    ListNode_t* node = list -> head;
    while (node != NULL) {
        char* item_str = NULL;
        if (item_stringify == NULL) {
            item_str = ptr_stringify(node -> data);
        } else {
            item_str = item_stringify(node -> data);
        }
        if (item_str == NULL) {
            item_str = calloc(sizeof(char), strlen(ITEM_NULL) + 1);
            memcpy(item_str, ITEM_NULL, sizeof(char) * strlen(ITEM_NULL));
        }

        if (node -> next == NULL) {
            const int old_length = items == NULL ? 0 :strlen(items);
            items = LinkedList_resize_items_str(items, item_str, 1);
            sprintf(items + old_length, "%s", item_str);
        } else {
            if (node == list -> head) {
                items = LinkedList_resize_items_str(items, item_str, 3);
                sprintf(items, "%s, ", item_str);
            } else {
                const int old_length = items == NULL ? 0 :strlen(items);
                items = LinkedList_resize_items_str(items, item_str, 3);
                sprintf(items + old_length, "%s, ", item_str);
            }
        }

        free(item_str);
        node = node -> next;
    }

    char* result = NULL;
    if (items == NULL) {
        result = calloc(sizeof(char), strlen(TEMPLATE_LINKED_LIST_EMPTY) + 1);
        sprintf(result, TEMPLATE_LINKED_LIST_EMPTY);
    } else {
        result = calloc(sizeof(char), strlen(TEMPLATE_LINKED_LIST) - 2 + strlen(items) + 1);
        sprintf(result, TEMPLATE_LINKED_LIST, items);
        free(items);
    }
    return result;
}



int compare_node(
    const ListNode_t* const node1, const ListNode_t* const node2,
    int (*item_compare)(const void* const item1, const void* const item2)
) {
    if (node1 == NULL && node2 != NULL) {
        return 1;
    } else if (node1 != NULL && node2 == NULL) {
        return -1;
    } else if (node1 == node2) {
        return 0;
    }
    if (item_compare == NULL) {
        if (node1 -> data > node2 -> data) {
            return 1;
        } else if(node1 -> data < node2 -> data) {
            return -1;
        }
    } else {
        int res = item_compare(node1 -> data, node2 -> data);
        if (res != 0) {
            return res;
        }
    }
    return 0;
}

int LinkedList_compare(
    const void* const p_list1, const void* const p_list2,
    int (*item_compare)(const void* const item1, const void* const item2)
) {
    if (p_list1 == NULL && p_list2 != NULL) {
        return 1;
    } else if (p_list1 != NULL && p_list2 == NULL) {
        return -1;
    }
    if (p_list1 == p_list2) {
        return 0;
    }
    const LinkedList_t* const list1 = (LinkedList_t*) p_list1;
    const LinkedList_t* const list2 = (LinkedList_t*) p_list2;

    if (list1 -> size != list2 -> size) {
        return list1 -> size > list2 -> size ? 1 : -1;
    }
    ListNode_t* node1 = list1 -> head;
    ListNode_t* node2 = list2 -> head;
    while (node1 != NULL && node2 != NULL) {
        int res = compare_node(node1, node2, item_compare);
        if (res != 0) {
            return res;
        }
        node1 = node1 -> next;
        node2 = node2 -> next;
    }

    return 0;
}
