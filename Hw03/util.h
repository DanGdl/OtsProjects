/*
 * linked_list.h
 *
 *  Created on: Nov 23, 2020
 *      Author: max
 */
#ifndef UTIL_H_
#define UTIL_H_

#include <strings.h>

size_t hash_string(const char* const value);

size_t hash_number_iteraction(size_t hash);

size_t hash_number_end(size_t hash);

size_t hash_number(const void* const value, size_t size);

char* ptr_stringify(const void* const ptr);

#endif
