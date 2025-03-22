#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "snode.h"
#include "slist.h"
#include "htable.h"

/**
 * @brief compare two key-value pairs by key.
 * 
 * @param a 
 * @param b 
 * @return int 
 */
int compare_key(const void *a, const void *b) {
	if (strcmp((char *)a, (char *)b) == 0){
		return 0;
	}
	return 1;
}

int compare_key_struct(const void *a, const void *b) {
	if (strcmp((((struct kv_pair *)a)->key), (char *)b) == 0){
		return 0;
	}
	return 1;
}

// djb2 hash function for a string
// does not need to be modified
int hash(const char *key) {
	int hash = 5381;
	for (size_t i = 0; key[i] != '\0'; i++) {
	
		hash = ((hash << 5) + hash) + key[i];
	}
	return hash;
}

struct htable *htable_create(uint32_t size) {
	// Allocate memory for hashmap struct
	struct htable *hashmap = (struct htable *) malloc(sizeof(struct htable));
	// Allocate memory for table of pointers in hashmap
	// table will need size * number of pointers = size * (struct slist*)
	hashmap->table = (struct slist **) malloc(size * sizeof(struct slist*));
	// Create lists in table array (allocate memory for each list)
	for (uint32_t i = 0; i < size; i++) {
		hashmap->table[i] = slist_create();
	}
	hashmap->size = size;
	hashmap->num_elems = 0;
	return hashmap;
}

// Destroys overarching structures
void htable_destroy(struct htable *ht) {	
	
	// Deallocate all lists
	for (uint32_t i = 0; i < ht->size; i++) {
		struct slist *list = ht->table[i];
		slist_destroy(list, 1);
	}

	// Deallocate table of pointers
	free(ht->table);

	// Dealocate htable struct
	free(ht);

	return;
}

void* htable_find(struct htable *ht, const char *key) {
	// Hash the key
	int key_hash = hash(key) % ht->size;

	// Find corresponding bucket in hash table
	struct slist *bucket = ht->table[key_hash];

	struct kv_pair *data = (struct kv_pair *) slist_find_value(bucket, (void *)key, compare_key_struct);

	if (data != NULL) {
		void *r_val = data->value;

		if (r_val != NULL) {
			return r_val;
		}
	}
	// Value not found so return NULL
	return NULL;
}

void* htable_del(struct htable *ht, const char *key) {
	// If key does not exists, then return NULL
	if (htable_find(ht, key) == NULL) {
		return NULL;
	} 

	// Hash the key
	int key_hash = hash(key) % ht->size;

	// Find corresponding bucket in hash table
	struct slist *bucket = ht->table[key_hash];

	struct kv_pair *data = (struct kv_pair *) slist_remove_value(bucket, (void *)key, compare_key_struct);


	if (data != NULL) {
		void *r_val = data->value;
		free(data);
		// Update count
		ht->num_elems--;

		if (r_val != NULL) {
			return r_val;
		}
	}

    return NULL; // Key was not found (shouldn't happen due to initial check)
}

bool htable_insert(struct htable *ht, const char *key, void *value) {
	// If key exists, then return false
	if (htable_find(ht, key) != NULL) {
		return false;
	} 

	// Create kv pair
	struct kv_pair *data = (struct kv_pair *) malloc(sizeof(struct kv_pair));
	data->key = key;
	data->value = value;
	
	// Hash the key
	int key_hash = hash(key) % ht->size;

	// Find corresponding bucket in hash table
	struct slist *bucket = ht->table[key_hash];

	// Add to back of bucket/list
	slist_add_back(bucket, data);

	// Increase element count
	ht->num_elems++;

	// If successful, return true
	return true;
}

void* htable_update(struct htable *ht, const char *key, void *value){
	// If key does not exists, then return false
	if (htable_find(ht, key) == NULL) {
		return false;
	} 

	// Hash the key
	int key_hash = hash(key) % ht->size;

	// Find corresponding bucket in hash table
	struct slist *bucket = ht->table[key_hash];

	// Find node and then update it's value
	struct snode *node = bucket->front;
	while (node != NULL) {
		struct kv_pair *pair = (struct kv_pair *)node->data;
		if (compare_key(pair->key, key) == 0) {
			// Inefficient, but need it to pass the test
			void *r_val = pair->value;
			pair->value = value;
			return r_val;
		}
		node = node->next;
	}

	// Node not found, so return NULL
	return NULL;
}

uint32_t htable_num_elems(struct htable *ht) {
	return ht->num_elems;
}

void **htable_values(struct htable *ht) {
	// Create an array to store values with size of num_elems
	void **arr = malloc(ht->num_elems * sizeof(void *));
	uint32_t idx = 0;

	// Go through every bucket and the nodes, saving the kv pair in the arr
	for (uint32_t i = 0; i < ht->size; i++) {
		struct slist *l = ht->table[i];
		struct snode *node = l->front;
		while (node != NULL) {
			arr[idx] = ((struct kv_pair *)node->data)->value;
			idx++;
			node = node->next;
		}
	}

	return arr;
}

struct htable *htable_dupe(struct htable *ht) {
    // Allocate memory for new hash table
    struct htable *hashmap = htable_create(ht->size);

    // Duplicate each bucket in the table
    for (uint32_t i = 0; i < ht->size; i++) {

        struct slist *orig_bucket = ht->table[i];
        struct snode *node = orig_bucket->front;

		// Go through all nodes in the given bucket in original htable and copy them
        while (node != NULL) {
			// Get original kv pair
            struct kv_pair *orig_kv = (struct kv_pair *)node->data;

            htable_insert(hashmap, orig_kv->key, orig_kv->value);

            node = node->next;
        }
    }

    return hashmap;
}


struct htable_iter* htable_create_iter(struct htable *ht) {
	struct htable_iter *iter = (struct htable_iter *) malloc(sizeof(struct htable_iter));
	iter->ht = ht;
	iter->index = 0;
	iter->current = NULL;

	return iter;
}


void htable_destroy_iter(struct htable_iter *iter) {
	if (iter) {
		free(iter);
	}
	return;
}


struct kv_pair *htable_iter_next(struct htable_iter *iter) {
    if (!iter || !iter->ht) {
        return NULL;  
    }

    struct htable *ht = iter->ht;
    struct slist **table = ht->table;

    // If the current node is valid, move to the next node
    if (iter->current) {
        iter->current = iter->current->next;
		// If next node is NOT NULL, then return early
        if (iter->current) {
            return (struct kv_pair *)iter->current->data;
        }
    }

    // Move to the next non-empty bucket (ONLY RUNS WHEN current is NULL)
    while (iter->index < ht->size) {
		// Index contains the index of the next bucket (not the same one in previous code block)
        struct slist *bucket = table[iter->index];
        if (bucket && bucket->front) {
            iter->current = bucket->front;
            iter->index++;  // Move index forward for next call/time this while loop runs
            return (struct kv_pair *)iter->current->data; // valid node so return early
        }
        iter->index++;  // Move to the next bucket for next call/time this while loop runs
    }

    return NULL;  // No more elements
}