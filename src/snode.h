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

#ifndef _SNODE_H_
#define _SNODE_H_

/**
 * Node in a singly-linked list.
 */
struct snode {
  struct snode *next;
  void *data;
};

/**
 * Allocates a new snode leaving the pointer to data as null.
 *
 * @return pointer to a new snode
 */
struct snode *snode_create();

/**
 * Sets the pointer to data with the value passed in.
 *
 * @param n pointer to the snone
 * @param ptr pointer to a generic data type to store in snode
 */
void snode_setdata(struct snode *n, void *ptr);

/**
 * Deallocates the memory associated with the snode and returns the pointer to the data.
 */
void *snode_destroy(struct snode *n);

#endif /* _SNODE_H_ */
