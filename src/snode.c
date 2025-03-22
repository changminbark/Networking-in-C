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
#include "snode.h"

struct snode *snode_create() {
	
	struct snode *node;
	
	node = (struct snode *) malloc(sizeof(struct snode));
	node->data = NULL;
	node->next = NULL;

	return node;
}

void snode_setdata(struct snode *n, void *ptr) {

	n->data = ptr;

	return;
}

void *snode_destroy(struct snode *n) {

	void *r_val = n->data;
	free(n);

	return r_val;
}
