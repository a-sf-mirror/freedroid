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
 * \brief Initializes a dynamic array to a base capacity
 * \param array Dynamic array.
 * \param membernum Number of members to allocate.
 * \param membersize Size of a member.
 */
void dynarray_init(struct dynarray *array, int membernum, size_t membersize)
{
	if (membernum) {
		array->arr = calloc(membernum, membersize);
		array->capacity = membernum;
		array->size = 0;
	} else {
		ErrorMessage(__FUNCTION__, "membernum is 0, this is illegal.\n", PLEASE_INFORM, IS_WARNING_ONLY);
	}
}

/**
 * \brief Allocates a dynamic array structure and initializes it to a base capacity.
 * \param membernum Number of member to allocate memory for.
 * \param membersize Size of a member.
 * \return Pointer to the freshly allocated dynamic array.
 */
struct dynarray *dynarray_alloc(int membernum, size_t membersize)
{
	struct dynarray *d = MyMalloc(sizeof(struct dynarray));
	dynarray_init(d, membernum, membersize);
	return d;
}

/**
 * \brief Reallocates a dynamic array.
 * \param array Dynamic array.
 * \param membernum Number of members to allocate, or zero to free.
 * \param membersize Size of a member.
 */
void dynarray_resize(struct dynarray *array, int membernum, size_t membersize)
{
	array->arr = realloc(array->arr, membernum * membersize);
	array->capacity = membernum;
}

/**
 * \brief Frees the contents of the dynamic array and sets the size to zero.
 * \param array Dynamic array.
 */
void dynarray_free(struct dynarray *array)
{
	free(array->arr);
	array->arr = NULL;
	array->size = 0;
	array->capacity = 0;
}

/**
 * \brief Add an element to a dynamic array. This function will extend the array capacity as required.
 * \param data Data to add.
 * \param membersize Size of the data to copy into the array.
 */
void dynarray_add(struct dynarray *array, void *data, size_t membersize)
{
	array->size++;

	if (array->size >= array->capacity) {
		dynarray_resize(array, array->size * 2, membersize);
	}

	memcpy(array->arr + membersize * (array->size - 1), data, membersize);
}
