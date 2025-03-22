/*
 * Copyright (c) 2024 Bucknell University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: L. Felipe Perrone (perrone@bucknell.edu)
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "snode.h"
#include "slist.h"

struct slist *
slist_create() {

    struct slist *list = (struct slist *) malloc(sizeof(struct slist));
    list->front = NULL;
    list->back = NULL;
    list->counter = 0;

    return list;
}

void 
slist_add_front(struct slist *l, void *ptr) {

    // Create node and set data
    struct snode *node = snode_create();
    snode_setdata(node, ptr);

    // Check if initially empty list, then assign back to node
    if (l->front == NULL) {
        l->back = node;
    }
    
    // Reassign list pointer and node's next pointer
    node->next = l->front;
    l->front = node;
    l->counter++;

#ifdef DEBUG
    printf("counter= %d, %s\n", l->counter, (char *) ptr);
    printf("front= %s\n", (char *) l->front->data);
    printf("back= %s\n\n", (char *) l->back->data);
#endif /* DEBUG */

    return;
}

void 
slist_add_back(struct slist *l, void *ptr) {

    // Create node and set data
    struct snode *node = snode_create();
    snode_setdata(node, ptr);

    // If list is initially empty (no back)
    if (l->back == NULL) {
        l->front = node;
        l->back = node;
    } else {
    // Reassign list pointer and node's next pointer
        l->back->next = node;
        l->back = node;
    }
    l->counter++;

#ifdef DEBUG
    printf("counter= %d, %s\n", l->counter, (char *) ptr);
    printf("front= %s\n", (char *) l->front->data);
    printf("back= %s\n\n", (char *) l->back->data);
#endif /* DEBUG */

    return;
}

void **
slist_to_array(struct slist *l) {
    // Allocate an array of enough size for number of strings
    void **arr = malloc(sizeof(void *) * l->counter);
    if (arr == NULL) {
        return NULL;  // If memory allocation fails, return NULL
    }

    // Iterate through every string and save into array
    struct snode *node = l->front;
    int i = 0;
    
    while (node != NULL) {
        arr[i] = node->data;
        node = node->next;
        i++;
    }

    return arr;
}

uint32_t 
slist_num_elems(struct slist *l) {
    return (l->counter);
}

void* slist_find_value(struct slist *l, void *value, int (*comparator)(const void*, const void*)) {
    if (!l) {
        perror("list is not valid\n");
        return NULL;
    }

    struct snode *node = l->front;

    while (node != NULL) {
        // If the values are the same, then return
        if (comparator(node->data, value) == 0) {
            return node->data;
        }
        node = node->next;
    }

    return NULL;
}

void* slist_remove_value(struct slist *l, void *value, int (*comparator)(const void*, const void*)) {
    // Look for key in list
    struct snode *node = l->front;
    struct snode *prev = NULL;

    while (node != NULL) {

        if (comparator(node->data, value) == 0) {
            // Key found, remove from list
            if (prev == NULL) {
                // If it's the first node, update front
                l->front = node->next;
            } else {
                // Skip the node in the linked list
                prev->next = node->next;
            }

			void *r_val = node->data;
            // Free node
            snode_destroy(node);

            // Decrease element count
            l->counter--;

			// Return the kv_pair associated with the deleted key to be deleted
            return r_val; 
        }

        prev = node;
        node = node->next;
    }

    return NULL; // Key was not found (shouldn't happen due to initial check)
}

void slist_destroy(struct slist *l, int free_node_data) {
    if (!l) return;

    struct snode *node = l->front;

    while (node != NULL) {
        struct snode *next = node->next;

        // Free the node
        void *data = snode_destroy(node);

        // Delete data if free_node_data is non zero
        if (free_node_data != 0) {
            free(data);
        }

        node = next;
    }

    // Free the list structure
    free(l);

    return;
}

struct slist *slist_dupe(struct slist *l) {
    if (!l) return NULL;  // Handle NULL input

    struct slist *new_list = (struct slist *) malloc(sizeof(struct slist));
    if (!new_list) return NULL;  // Handle memory allocation failure

    new_list->front = NULL;
    new_list->back = NULL;
    new_list->counter = 0;

    struct snode *node = l->front;
    while (node) {
        // Create a new node but do not duplicate the data
        struct snode *new_node = snode_create();
        snode_setdata(new_node, node->data);  // Shallow copy: reuse the same data pointer

        // Add to new list
        if (new_list->back == NULL) {
            new_list->front = new_node;
            new_list->back = new_node;
        } else {
            new_list->back->next = new_node;
            new_list->back = new_node;
        }

        node = node->next;
        new_list->counter++;
    }

    return new_list;
}



