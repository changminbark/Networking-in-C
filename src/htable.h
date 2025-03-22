/**
 * @file htable.h

 * @brief Hash table implementation.
 * @version 0.2
 * @date 2025-01-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _HTABLE_H_

#define _HTABLE_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "snode.h"
#include "slist.h"

struct htable {
	struct slist **table;
	uint32_t size;
	uint32_t num_elems;
};

struct htable_iter {
	struct htable *ht;
	uint32_t index;
	struct snode *current;
};

/**
 * @brief a key-value pair is created for all inserted elements and added to the corresponding singly linked list.
 * when searching for a key, the hash table will first find the corresponding singly linked list and then search for the key in that list.
 * the compare_key function is used to compare two keys.
 */
struct kv_pair {
	const char *key;
	void *value;
};

/**
 * @brief Create a hash table object.
 * 
 * @param size The size of the hash table.
 * @return struct htable* A pointer to the hash table.
 */
struct htable *htable_create(uint32_t size);

/**
 * @brief Destroy the hash table object.
 * 
 * @param ht A pointer to the hash table.
 */
void htable_destroy(struct htable *ht);

/**
 * @brief Insert a NEW key-value pair into the hash table.
 * 
 * @param ht A pointer to the hash table.
 * @param key The key.
 * @param value The value.
 * @return bool True if the key-value pair was inserted, false otherwise.
 * if the key already exists, false is returned and the value is NOT updated.
 */
bool htable_insert(struct htable *ht, const char *key, void *value);

/**
 * @brief update the value of an existing key in the hash table.
 * 
 * @param ht 
 * @param key 
 * @param value 
 * @return void* the old value or NULL if the key was not found.
 */
void* htable_update(struct htable *ht, const char *key, void *value);

/**
 * @brief find a key in the hash table and return the value or NULL if not found.
 * 
 * @param ht 
 * @param key 
 * @return void* 
 */
void*
htable_find(struct htable *ht, const char *key);

/**
 * @brief find the key and remove it from the hash table.
 * 
 * @param ht 
 * @param key 
 * @return void* 
 */
void* 
htable_del(struct htable *ht, const char *key);


/**
 * @brief returns the number of elements in the hash table.
 * 
 * @param ht 
 * @return uint32_t 
 */
uint32_t htable_num_elems(struct htable *ht);

/**
 * @brief return an array of the values stored in the hash table.
 * 
 * @param ht 
 * @return void** 
 */
void **htable_values(struct htable *ht);

/**
 * @brief duplicate the hash table. (shallow copy).
 * 
 * @param ht 
 * @return struct htable* 
 */
struct htable *htable_dupe(struct htable *ht);


/**
 * @brief create an iterator for the hash table. 
 */
struct htable_iter* htable_create_iter(struct htable *ht);

/**
 * @brief destroy the iterator.
 */
void htable_destroy_iter(struct htable_iter *iter);

/**
 * @brief return the next key-value pair in the hash table or NULL if there are no more pairs.
 */
struct kv_pair *htable_iter_next(struct htable_iter *iter);

#endif /* _HTABLE_H_ */