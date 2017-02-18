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

//===================================================================
// Dynarray functions
//===================================================================

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
	}
	array->capacity = membernum;
	array->size = 0;
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

void dynarray_cpy(struct dynarray *to, struct dynarray *from, size_t membersize)
{
	to->arr = MyMalloc(from->capacity * membersize);
	memcpy(to->arr, from->arr, from->capacity * membersize);
	to->capacity = from->capacity;
	to->size = from->size;
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
	array->size = 0;
	array->capacity = 0;
}

/**
 * \brief Reallocates a dynamic array.
 *
 * \param array           Pointer to the dynarray to resize.
 * \param membernum       Number of members to allocate, or zero to free.
 * \param membersize      Size of a member.
 */
static void dynarray_resize(struct dynarray *array, int membernum, size_t membersize)
{
	void *buffer = realloc(array->arr, membernum * membersize);
	if (!buffer) {
		error_message(__FUNCTION__,
		              "Not enough memory to realloc a dynarray (requested size: " SIZE_T_F ")",
		              IS_FATAL, membernum * membersize);
	}
	array->arr = buffer;
	array->capacity = membernum;
}

/**
 * \brief Store an element at a given slot of a dynarray.
 *
 * \param array      Pointer to the dynarray to which the element is to be added.
 * \param index      Index of the slot to fill with the data.
 * \param data       Pointer to the data to add.
 * \param membersize Size of the data to copy into the array.
 */
static void dynarray_store_data(struct dynarray *array, int index, void *data, size_t membersize)
{
	// cppcheck-suppress arithOperationsOnVoidPointer
	memcpy(array->arr + membersize * index, data, membersize);
}

/**
 * \brief Add an element to a dynamic array. This function will extend the array capacity as required.
 *
 * \param array      Pointer to the dynarray to which the element is to be added.
 * \param data       Pointer to the data to add.
 * \param membersize Size of the data to copy into the array.
 */
void dynarray_add(struct dynarray *array, void *data, size_t membersize)
{
	if (array->size >= array->capacity) {
		// If the dynarray was not yet allocated, allocate a default capacity
		// of 8 elements.
		dynarray_resize(array, (array->capacity) ? array->capacity * 2 : 8, membersize);
	}

	// Copy the data at the tail of the dynarray
	dynarray_store_data(array, array->size, data, membersize);

	array->size++;
	return;
}

/**
 * \brief Remove an element from a dynamic array.
 *
 * \details The array is 'compressed' to fill the hole.
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

	// If we are removing the last element, decrementing the array size is
	// enough

	int remove_last = (index == array->size - 1);
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

//===================================================================
// Sparse dynarray functions
//===================================================================

/**
 * \brief Initializes a sparse dynamic array to a base capacity
 *
 * \param array       Pointer to the sparse dynarray to initiliaze.
 * \param membernum   Number of members to allocate.
 * \param membersize  Size of a member.
 */
void sparse_dynarray_init(struct sparse_dynarray *array, int membernum, size_t membersize)
{
	// downcasting to a dynarray to initialize the 'embedded' dynarray
	dynarray_init((struct dynarray *)array, membernum, membersize);

	// sparse dynarray specific code

	if (membernum) {
		array->used_members = calloc(membernum, sizeof(array->used_members[0]));
	} else {
		array->used_members = NULL;
	}
}

/**
 * \brief Allocates a sparse dynamic array structure and initializes it to a base capacity.
 *
 * \param membernum   Number of member to allocate memory for.
 * \param membersize  Size of a member.
 *
 * \return Pointer to the freshly allocated dynamic array.
 */
struct sparse_dynarray *sparse_dynarray_alloc(int membernum, size_t membersize)
{
	struct sparse_dynarray *d = MyMalloc(sizeof(struct sparse_dynarray));
	sparse_dynarray_init(d, membernum, membersize);
	return d;
}

/**
 * \brief Frees the contents of a sparse dynamic array and sets the size to zero.
 *
 * \param array  Pointer to the sparse dynarray to free.
 */
void sparse_dynarray_free(struct sparse_dynarray *array)
{
	// downcasting to a dynarray to free the 'embedded' dynarray
	dynarray_free((struct dynarray *)array);

	// sparse_dynarray specific code

	free(array->used_members);
	array->used_members = NULL;
}

/**
 * \brief Reallocates a sparse dynamic array.
 *
 * \param array           Pointer to the sparse dynarray to resize.
 * \param membernum       Number of members to allocate, or zero to free.
 * \param membersize      Size of a member.
 */
static void sparse_dynarray_resize(struct sparse_dynarray *array, int membernum, size_t membersize)
{
	// downcasting to a dynarray to resize the 'embedded' dynarray
	dynarray_resize((struct dynarray *)array, membernum, membersize);

	// sparse_dynarray specific code

	void *buffer = realloc(array->used_members, membernum * sizeof(array->used_members[0]));
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

/**
 * \brief Add an element to a sparse dynamic array. This function will extend the array capacity as required.
 *
 * \details The element is added to the first available slot.
 *          If the sparse dynarray is full, the element is added at the end of the sparse dynarray.
 *
 * \param array      Pointer to the sparse dynarray to which the element is to be added.
 * \param data       Pointer to the data to add.
 * \param membersize Size of the data to copy into the array.
 */
void sparse_dynarray_add(struct sparse_dynarray *array, void *data, size_t membersize)
{
	int slot_index = -1;

	// Check if a slot is available
	int i;
	for (i = 0; i < array->size; i++) {
		if (!array->used_members[i]) {
			slot_index = i;
			break;
		}
	}

	// If no slot is available in the sparse dynarray, the element is to be added
	// at the end of the array, and we have to check that there is enough room.
	if (slot_index == -1) {
		slot_index = array->size;
		if (array->size >= array->capacity) {
			// If the sparse dynarray was not yet allocated, allocate a default capacity
			// of 8 elements.
			sparse_dynarray_resize(array, (array->capacity) ? array->capacity * 2 : 8, membersize);
		}
		array->size++;
	}

	// Copy the data in the free slot and mark the slot as being used
	dynarray_store_data((struct dynarray *)array, slot_index, data, membersize);
	array->used_members[slot_index] = TRUE;

	return;
}

/**
 * \brief Remove an element from a sparse dynamic array.
 *
 * \details The slot is marked as available for future insertion of a new
 *          element. If there are trailing unused slots, the size of the array
 *          is reduced to the last used slot.
 *
 * \param array       Pointer to the sparse dynarray to use
 * \param index       Index of the element to remove
 * \param membersize  Size of the element to remove
 */
void sparse_dynarray_del(struct sparse_dynarray *array, int index, size_t membersize)
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

	array->used_members[index] = 0;

	// Check if we are removing the last element of the array
	// If so, we decrement the size of the array up to the last used slot.
	int remove_last = (index == array->size - 1);
	if (remove_last) {
		do {
			array->size--;
		} while (array->size > 0 && array->used_members[array->size - 1] == 0);
	}
}

/**
 * \brief Get a pointer to an element in a sparse_dynamic array.
 *
 * \param array       Pointer to the array to use
 * \param index       Index of the element
 * \param membersize  Size of the elements in the array
 *
 * \return A pointer to the element
 */
void *sparse_dynarray_member(struct sparse_dynarray *array, int index, size_t membersize)
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
int sparse_dynarray_member_used(struct sparse_dynarray *array, int index)
{
	return array->used_members[index];
}
