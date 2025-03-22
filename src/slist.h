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

#ifndef _SLIST_H_
#define _SLIST_H_

#include <stdint.h>
#include <stdbool.h>
#include "snode.h"

/**
 * Singly-linked list.
 */
struct slist {
  struct snode *front;
  struct snode *back;
  uint32_t counter;
};

/**
 * Allocates new dlist dynamically.
 * 
 * @return pointer to the list.
 */
struct slist *slist_create();

/** 
 * Inserts new node in slist after the last node.
 *
 * @param l pointer to the list (non-NULL)
 * @param ptr pointer to generic data to store in new list node
 *
 */
void slist_add_back(struct slist *l, void *ptr);

/** 
 * Inserts new node in slist before the first node.
 *
 * @param l pointer to the list (non-NULL)
 * @param ptr pointer to generic data to store in new list node
 *
 */
void slist_add_front(struct slist *l, void *ptr);


/**
 * Convert the singly-linked list to an array of snode pointers.
 */
void ** 
slist_to_array(struct slist *l);

/** 
 * Returns the number of elements in the list (nodes).
 *
 * @param l pointer to the list (non-NULL)
 */
uint32_t slist_num_elems(struct slist *l);

/**
 * @brief Search the list for a value using a comparator function.
 * @return void* pointer to the value found or NULL if not found.
 */
void* slist_find_value(struct slist *l, void *value, int (*comparator)(const void*, const void*));

/**
 * @brief Remove the first node from the list that contains the value using the comparator function.
 * @return pointer to the data in the removed node or NULL if not found.
 */
void *slist_remove_value(struct slist *l, void *value, int (*comparator)(const void*, const void*));

/**
 * Deallocates and destroys the list and all nodes.
 * @param free_node_data if non-zero, free the data in each node.
 */
void slist_destroy(struct slist *l, int free_node_data);

/**
 * @brief Duplicate the singly-linked list. (shallow copy).
 */
struct slist *slist_dupe(struct slist *l);

#endif /* _SLIST_H_ */
