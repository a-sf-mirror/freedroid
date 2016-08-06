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
 *
 * \param array       Pointer to the dynarray to initialize.
 * \param membernum   Number of members to allocate.
 * \param membersize  Size of a member.
 */
void dynarray_init(struct dynarray *array, int membernum, size_t membersize)
{
	if (membernum) {
		array->arr = calloc(membernum, membersize);
	} else {
		array->arr = NULL;
		array->used_members = NULL;
	}
	array->capacity = membernum;
	array->size = 0;
	array->sparse = FALSE;
	array->used_members = NULL;
}

/**
 * \brief Initializes a sparse dynamic array to a base capacity
 *
 * \param array       Pointer to the sparse dynarray to initiliaze.
 * \param membernum   Number of members to allocate.
 * \param membersize  Size of a member.
 */
void sparse_dynarray_init(struct dynarray *array, int membernum, size_t membersize)
{
	dynarray_init(array, membernum, membersize);
	if (membernum) {
		array->used_members = calloc(membernum, sizeof(array->used_members[0]));
	} else {
		array->used_members = NULL;
	}
	array->sparse = TRUE;
}

/**
 * \brief Allocates a dynamic array structure and initializes it to a base capacity.
 *
 * \param membernum   Number of member to allocate memory for.
 * \param membersize  Size of a member.
 *
 * \return Pointer to the freshly allocated dynamic array.
 */
struct dynarray *dynarray_alloc(int membernum, size_t membersize)
{
	struct dynarray *d = MyMalloc(sizeof(struct dynarray));
	dynarray_init(d, membernum, membersize);
	return d;
}

/**
 * \brief Allocates a sparse dynamic array structure and initializes it to a base capacity.
 *
 * \param membernum   Number of member to allocate memory for.
 * \param membersize  Size of a member.
 *
 * \return Pointer to the freshly allocated dynamic array.
 */
struct dynarray *sparse_dynarray_alloc(int membernum, size_t membersize)
{
	struct dynarray *d = MyMalloc(sizeof(struct dynarray));
	sparse_dynarray_init(d, membernum, membersize);
	return d;
}

/**
 * \brief Reallocates a dynamic array.
 *
 * \param array           Pointer to the dynarray to resize.
 * \param membernum       Number of members to allocate, or zero to free.
 * \param membersize      Size of a member.
 */
void dynarray_resize(struct dynarray *array, int membernum, size_t membersize)
{
	void *buffer = realloc(array->arr, membernum * membersize);
	if (!buffer) {
		error_message(__FUNCTION__,
		              "Not enough memory to realloc a dynarray (requested size: " SIZE_T_F ")",
		              IS_FATAL, membernum * membersize);
	}
	array->arr = buffer;

	if (array->sparse) {
		buffer = realloc(array->used_members, membernum * sizeof(array->used_members[0]));
		if (!buffer) {
			error_message(__FUNCTION__,
			              "Not enough memory to realloc the used_members of a dynarray (requested size: " SIZE_T_F ")",
			              IS_FATAL, membernum * sizeof(array->used_members[0]));
		}
		array->used_members = buffer;
		// set new slots as unused (not really needed since they are after the
		// last used slot, but it can prevent a potential bug)
		memset(&array->used_members[array->size], 0, (membernum - array->size) * sizeof(array->used_members[0]));
	}

	array->capacity = membernum;
}

/**
 * \brief Frees the contents of the dynamic array and sets the size to zero.
 *
 * \param array  Pointer to the dynarray to free.
 */
void dynarray_free(struct dynarray *array)
{
	free(array->arr);
	array->arr = NULL;
	free(array->used_members);
	array->used_members = NULL;
	array->size = 0;
	array->capacity = 0;
}

/**
 * \brief Add an element to a dynamic array. This function will extend the array capacity as required.
 *
 * \details If the dynarray is a sparse one, the element is added to the first available slot.
 *          Else, or if the sparse dynarray is full, the element is added at the end the the dynarray.
 *
 * \param array      Pointer to the dynarray to which the element is to be added.
 * \param data       Pointer to the data to add.
 * \param membersize Size of the data to copy into the array.
 */
void dynarray_add(struct dynarray *array, void *data, size_t membersize)
{
	int member_index = -1;

	// Check if a slot is available, if the dynarray is a sparse one

	if (array->sparse) {
		int i;
		for (i = 0; i < array->size; i++) {
			if (!array->used_members[i]) {
				member_index = i;
				break;
			}
		}
	}

	// If no slot is available in a sparse dynarray or if the dynarray is not a
	// sparse one, add the element at the end of the array

	if (member_index == -1) {
		member_index = array->size;
		if (array->size >= array->capacity) {
			// If the dynarray was not yet allocated, allocate a default capacity
			// of 8 elements.
			dynarray_resize(array, (array->capacity) ? array->capacity * 2 : 8, membersize);
		}
		array->size++;
	}

	// cppcheck-suppress arithOperationsOnVoidPointer
	memcpy(array->arr + membersize * member_index, data, membersize);

	if (array->sparse)
		array->used_members[member_index] = TRUE;

	return;
}

/**
 * \brief Remove an element from a dynamic array.
 *
 * \details If the dynarray is a sparse one, the slot is marked as available for
 *          future insertion of a new element. If there are trailing unused slots,
 *          the size of the array is reduced to the last used slot.
 *          If the dynarray is not a sparse one, the array is 'compressed'.
 *          This operation can be costly in terms of memory traffic.
 *
 * \param array       Pointer to the dynarray to use
 * \param index       Index of the element to remove
 * \param membersize  Size of the element to remove
 */
void dynarray_del(struct dynarray *array, int index, size_t membersize)
{
	// Check some pre-conditions
	if (array->size == 0) {
		error_message(__FUNCTION__,
		              "Trying to delete a member of an empty dynarray.",
		              IS_FATAL);
	}
	if (index < 0 || index >= array->size) {
		error_message(__FUNCTION__,
		              "Out of scope member's index. Index: %d - Array size: %d",
		              IS_FATAL, index, array->size);
	}

	int remove_last = (index == array->size - 1);

	if (array->sparse) {
		array->used_members[index] = 0;

		// Check if we are removing the last element of the array
		// If so, we decrement the size of the array up to the last used slot.
		if (remove_last) {
			do {
				array->size--;
			} while (array->size > 0 && array->used_members[array->size - 1] == 0);
		}
	} else {

		// If we are removing the last element, decrementing the array size is
		// enough

		if (remove_last) {
			array->size--;
			return;
		}

		// else, we have to 'compress' the array

		// cppcheck-suppress arithOperationsOnVoidPointer
		void *addr = array->arr + membersize * index;
		// cppcheck-suppress arithOperationsOnVoidPointer
		void *next = addr + membersize;
		int nb = membersize * (array->size - index - 1);

		memmove(addr, next, nb);

		array->size--;
	}
}

/**
 * \brief Get a pointer to an element in a dynamic array.
 *
 * \param array       Pointer to the array to use
 * \param index       Index of the element
 * \param membersize  Size of the elements in the array
 *
 * \return A pointer to the element
 */
void *dynarray_member(struct dynarray *array, int index, size_t membersize)
{
	// cppcheck-suppress arithOperationsOnVoidPointer
	return array->arr + membersize * index;
}

/**
 * \brief Check if a slot of a sparse dynarray is in use.
 *
 * \param array       Pointer to the array to use
 * \param index       Index of the element
 *
 * \return TRUE if the slot is used, FALSE otherwise
 */
int sparse_dynarray_member_used(struct dynarray *array, int index)
{
	if (!array->sparse) {
		error_message(__FUNCTION__,
		              "This is not a sparse dynarray, something is really wrong in the code.\n"
		              "We cannot safely continue. Sorry.",
		              IS_FATAL);
	}
	return array->used_members[index];
}
