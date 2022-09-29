#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*
* print:
* 4 8 15 16 23 42
* 23 15
*/
int8_t data[] = { 4, 8, 15, 16, 23, 42 };
int data_length = sizeof(data) / sizeof(*data);


typedef struct LinkedList {
    uint8_t value;
    struct LinkedList* next;
} LinkedList_t;



void m(LinkedList_t* list);


void print_int(int64_t value) {
    if (value == 0) {
        return;
    }
    printf("%ld ", value);
}

int p(int8_t* value) {
    return (*value) & 1; // check is not-even
}

void add_element(int8_t value, LinkedList_t** list) {
    (*list) = malloc(sizeof(**list));
    if ((*list) == NULL) {
        exit(-1);
    }
    (*list) -> value = value;
    (*list) -> next = NULL;
}

void m(LinkedList_t* list) {
    if (list == NULL) {
        return;
    }
    print_int(list -> value);
    m(list -> next);
}

void f(int8_t* source_value, LinkedList_t** list) {
    if (source_value == NULL || *source_value == 0) {
        return;
    }
    f(source_value + 1, list);
    if (p(source_value)) {
        LinkedList_t** current = list;
        while(*current != NULL) {
            current = &((*current) -> next);
        }
        add_element(*source_value, current);
    }
}

int main(void) {
    int lengh = data_length;
    LinkedList_t* list = NULL;
    LinkedList_t** current = &list;
    do {
        add_element(data[lengh - 1], current);
        current = &((*current) -> next);
        lengh--;
    } while (lengh != 0);

    m(list);
    puts("");

    while (list != NULL) {
        LinkedList_t* node = list;
        list = list -> next;
        free(node);
    }

    f(data, &list);
    m(list);
    puts("");

    while (list != NULL) {
        LinkedList_t* node = list;
        list = list -> next;
        free(node);
    }
    return 0;
}
