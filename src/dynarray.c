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
 * \brief Allocates a dynamic array and fills it with zeros.
 * \param array Dynamic array.
 * \param membernum Number of members to allocate.
 * \param membersize Size of a member.
 */
void dynarray_alloc(struct dynarray *array, int membernum, size_t membersize)
{
	if (membernum)
	{
		array->arr = calloc(membernum, membersize);
		array->size = membernum;
	}
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
	array->size = membernum;
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
}
