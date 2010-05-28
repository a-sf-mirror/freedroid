/* 
 *
 *   Copyright (c) 2010 Ari Mustonen
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

/**
 * \brief Adds an upgrade socket to the item.
 * \param it Item.
 * \param type Upgrade socket type.
 * \param addon Upgrade item type or NULL.
 */
void create_upgrade_socket(item *it, int type, const char *addon)
{
	struct upgrade_socket socket; 

	// Initialize upgrade socket data.
	socket.type = type;
	socket.addon = NULL;
	if (addon) {
		socket.addon = strdup(addon);
	}

	// Append an upgrade socket to the socket array.
	dynarray_add((struct dynarray *) &it->upgrade_sockets, &socket, sizeof(struct upgrade_socket));
}

/**
 * \brief Deletes all upgrade sockets of an item.
 * \param it Item.
 */
void delete_upgrade_sockets(item *it)
{
	int i;
	for (i = 0 ; i < it->upgrade_sockets.size ; i++) {
		free(it->upgrade_sockets.arr[i].addon);
	}
	dynarray_free((struct dynarray *) &it->upgrade_sockets);
}

/**
 * \brief Copies upgrade sockets from an item to another.
 *
 * The destination item must have its sockets deleted prior to calling this.
 * Otherwise, the old sockets will be leaked.
 * \param srcitem Source item.
 * \param dstitem Destination item.
 */
void copy_upgrade_sockets(item *srcitem, item *dstitem)
{
	int i;

	// Allocate new upgrade sockets.
	int size = srcitem->upgrade_sockets.size;
	dynarray_init((struct dynarray *) &dstitem->upgrade_sockets, size, sizeof(struct upgrade_socket));

	// Duplicate socket data.
	for (i = 0 ; i < size ; i++) {
		dynarray_add((struct dynarray *) &dstitem->upgrade_sockets, &srcitem->upgrade_sockets.arr[i],
		              sizeof(struct upgrade_socket));
		if (srcitem->upgrade_sockets.arr[i].addon) {
			dstitem->upgrade_sockets.arr[i].addon = strdup(srcitem->upgrade_sockets.arr[i].addon);
		}
	}
}
