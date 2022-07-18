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



void m(int16_t** value);


void print_int(int64_t value) {
    if (value == 0) {
        return;
    }
    printf("%ld ", value);
}

int p(int8_t* value) {
    return (*value) & 1; // check is not-even
}

void add_element(int8_t first, int16_t** array) {
    (*array) = malloc(16);
    if ((*array) == NULL) {
        exit(-1);
    }
    (*array)[0] = first;
    (*array)[1] = 0;
}

void m(int16_t** value) {
    if (value == NULL || *value == 0) {
        return;
    }
    print_int(**value);
    m(value - 1);
}

void f(int8_t* source_value, int16_t** value) {
    if (source_value == NULL || *source_value == 0) {
        return;
    }
    f(source_value + 1, value);
    if (p(source_value)) {
        while (*value != NULL) {
            value = value - 1;
        }
        add_element(*source_value, value);
    }
}

int main(void) {
    int lengh = data_length;
    int16_t* array[data_length + 1];
    do {
        add_element(data[lengh - 1], &array[data_length - lengh + 1]);
        lengh--;
    } while (lengh != 0);

    m(array + data_length);
    puts("");

    for (int i = 0; i < data_length + 1; i++) {
        if (array[i] == NULL) {
            continue;
        }
        free(array[i]);
        array[i] = NULL;
    }

    f(data, array + data_length);
    m(array + data_length);
    puts("");

    for (int i = 0; i < data_length + 1; i++) {
        if (array[i] == NULL) {
            continue;
        }
        free(array[i]);
        array[i] = NULL;
    }
    return 0;
}
