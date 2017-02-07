/* =============================================================================
 *
 * list.c
 * -- Sorted singly linked list
 * -- Options: -DLIST_NO_DUPLICATES (default: allow duplicates)
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * For the license of bayes/sort.h and bayes/sort.c, please see the header
 * of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of kmeans, please see kmeans/LICENSE.kmeans
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of ssca2, please see ssca2/COPYRIGHT
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/mt19937ar.c and lib/mt19937ar.h, please see the
 * header of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/rbtree.h and lib/rbtree.c, please see
 * lib/LEGALNOTICE.rbtree and lib/LICENSE.rbtree
 * 
 * ------------------------------------------------------------------------
 * 
 * Unless otherwise noted, the following license applies to STAMP files:
 * 
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 * 
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 */


#include <stdlib.h>
#include <assert.h>
#include "list-stm.h"
#include "types.h"
#include "tm.h"

using namespace list_stm;

/* =============================================================================
 * DECLARATION OF TM_CALLABLE FUNCTIONS
 * =============================================================================
 */

TM_CALLABLE
static list_node_t*
TMfindPrevious (TM_ARGDECL  list_t* listPtr, void* dataPtr);

TM_CALLABLE
static void
TMfreeList (TM_ARGDECL  list_node_t* nodePtr);

TM_CALLABLE
void
TMlist_free (TM_ARGDECL  list_t* listPtr);

/* =============================================================================
 * compareDataPtrAddresses
 * -- Default compare function
 * =============================================================================
 */
static long
compareDataPtrAddresses (const void* a, const void* b)
{
    return ((long)a - (long)b);
}


/* =============================================================================
 * list_iter_reset
 * =============================================================================
 */
void
list_stm::list_iter_reset (list_iter_t* itPtr, list_t* listPtr)
{
    *itPtr = &(listPtr->head);
}


/* =============================================================================
 * TMlist_iter_reset
 * =============================================================================
 */
void
list_stm::TMlist_iter_reset (TM_ARGDECL  list_iter_t* itPtr, list_t* listPtr)
{
    TM_LOCAL_WRITE_P(*itPtr, &(listPtr->head));
}


/* =============================================================================
 * list_iter_hasNext
 * =============================================================================
 */
bool_t
list_stm::list_iter_hasNext (list_iter_t* itPtr, list_t* listPtr)
{
    return (((*itPtr)->nextPtr != NULL) ? TRUE : FALSE);
}


/* =============================================================================
 * TMlist_iter_hasNext
 * =============================================================================
 */
bool_t
list_stm::TMlist_iter_hasNext (TM_ARGDECL  list_iter_t* itPtr, list_t* listPtr)
{
    list_iter_t next = (list_iter_t)SLOW_PATH_SHARED_READ_P((*itPtr)->nextPtr);

    return ((next != NULL) ? TRUE : FALSE);
}


/* =============================================================================
 * list_iter_next
 * =============================================================================
 */
void*
list_stm::list_iter_next (list_iter_t* itPtr, list_t* listPtr)
{
    *itPtr = (*itPtr)->nextPtr;

    return (*itPtr)->dataPtr;
}


/* =============================================================================
 * TMlist_iter_next
 * =============================================================================
 */
void*
list_stm::TMlist_iter_next (TM_ARGDECL  list_iter_t* itPtr, list_t* listPtr)
{
    list_iter_t next = (list_iter_t)SLOW_PATH_SHARED_READ_P((*itPtr)->nextPtr);
    TM_LOCAL_WRITE_P(*itPtr, next);

    return next->dataPtr;
}


/* =============================================================================
 * allocNode
 * -- Returns NULL on failure
 * =============================================================================
 */
static list_node_t*
allocNode (void* dataPtr)
{
    list_node_t* nodePtr = (list_node_t*)malloc(sizeof(list_node_t));
    if (nodePtr == NULL) {
        return NULL;
    }

    nodePtr->dataPtr = dataPtr;
    nodePtr->nextPtr = NULL;

    return nodePtr;
}


/* =============================================================================
 * PallocNode
 * -- Returns NULL on failure
 * =============================================================================
 */
static list_node_t*
PallocNode (void* dataPtr)
{
    list_node_t* nodePtr = (list_node_t*)P_MALLOC(sizeof(list_node_t));
    if (nodePtr == NULL) {
        return NULL;
    }

    nodePtr->dataPtr = dataPtr;
    nodePtr->nextPtr = NULL;

    return nodePtr;
}


/* =============================================================================
 * TMallocNode
 * -- Returns NULL on failure
 * =============================================================================
 */
static list_node_t*
TMallocNode (TM_ARGDECL  void* dataPtr)
{
    list_node_t* nodePtr = (list_node_t*)TM_MALLOC(sizeof(list_node_t));
    if (nodePtr == NULL) {
        return NULL;
    }

    nodePtr->dataPtr = dataPtr;
    nodePtr->nextPtr = NULL;

    return nodePtr;
}


/* =============================================================================
 * list_alloc
 * -- If NULL passed for 'compare' function, will compare data pointer addresses
 * -- Returns NULL on failure
 * =============================================================================
 */
list_t*
list_stm::list_alloc (long (*compare)(const void*, const void*))
{
    list_t* listPtr = (list_t*)malloc(sizeof(list_t));
    if (listPtr == NULL) {
        return NULL;
    }

    listPtr->head.dataPtr = NULL;
    listPtr->head.nextPtr = NULL;
    listPtr->size = 0;

    if (compare == NULL) {
        listPtr->compare = &compareDataPtrAddresses; /* default */
    } else {
        listPtr->compare = compare;
    }

    return listPtr;
}


/* =============================================================================
 * Plist_alloc
 * -- If NULL passed for 'compare' function, will compare data pointer addresses
 * -- Returns NULL on failure
 * =============================================================================
 */
list_t*
list_stm::Plist_alloc (long (*compare)(const void*, const void*))
{
    list_t* listPtr = (list_t*)P_MALLOC(sizeof(list_t));
    if (listPtr == NULL) {
        return NULL;
    }

    listPtr->head.dataPtr = NULL;
    listPtr->head.nextPtr = NULL;
    listPtr->size = 0;

    if (compare == NULL) {
        listPtr->compare = &compareDataPtrAddresses; /* default */
    } else {
        listPtr->compare = compare;
    }

    return listPtr;
}


/* =============================================================================
 * TMlist_alloc
 * -- If NULL passed for 'compare' function, will compare data pointer addresses
 * -- Returns NULL on failure
 * =============================================================================
 */
list_t*
list_stm::TMlist_alloc (TM_ARGDECL  long (*compare)(const void*, const void*))
{
    list_t* listPtr = (list_t*)TM_MALLOC(sizeof(list_t));
    if (listPtr == NULL) {
        return NULL;
    }

    listPtr->head.dataPtr = NULL;
    listPtr->head.nextPtr = NULL;
    listPtr->size = 0;

    if (compare == NULL) {
        listPtr->compare = &compareDataPtrAddresses; /* default */
    } else {
        listPtr->compare = compare;
    }

    return listPtr;
}


/* =============================================================================
 * freeNode
 * =============================================================================
 */
static void
freeNode (list_node_t* nodePtr)
{
    free(nodePtr);
}


/* =============================================================================
 * PfreeNode
 * =============================================================================
 */
static void
PfreeNode (list_node_t* nodePtr)
{
    P_FREE(nodePtr);
}


/* =============================================================================
 * TMfreeNode
 * =============================================================================
 */
static void
TMfreeNode (TM_ARGDECL  list_node_t* nodePtr)
{
    SLOW_PATH_FREE(nodePtr);
}


/* =============================================================================
 * freeList
 * =============================================================================
 */
static void
freeList (list_node_t* nodePtr)
{
    if (nodePtr != NULL) {
        freeList(nodePtr->nextPtr);
        freeNode(nodePtr);
    }
}


/* =============================================================================
 * PfreeList
 * =============================================================================
 */
static void
PfreeList (list_node_t* nodePtr)
{
    if (nodePtr != NULL) {
        PfreeList(nodePtr->nextPtr);
        PfreeNode(nodePtr);
    }
}


/* =============================================================================
 * TMfreeList
 * =============================================================================
 */
static void
TMfreeList (TM_ARGDECL  list_node_t* nodePtr)
{
    if (nodePtr != NULL) {
        list_node_t* nextPtr = (list_node_t*)SLOW_PATH_SHARED_READ_P(nodePtr->nextPtr);
        TMfreeList(TM_ARG  nextPtr);
        TMfreeNode(TM_ARG  nodePtr);
    }
}


/* =============================================================================
 * list_free
 * =============================================================================
 */
void
list_stm::list_free (list_t* listPtr)
{
    freeList(listPtr->head.nextPtr);
    free(listPtr);
}


/* =============================================================================
 * Plist_free
 * =============================================================================
 */
void
list_stm::Plist_free (list_t* listPtr)
{
    PfreeList(listPtr->head.nextPtr);
    P_FREE(listPtr);
}


/* =============================================================================
 * TMlist_free
 * =============================================================================
 */
void
list_stm::TMlist_free (TM_ARGDECL  list_t* listPtr)
{
    list_node_t* nextPtr = (list_node_t*)SLOW_PATH_SHARED_READ_P(listPtr->head.nextPtr);
    TMfreeList(TM_ARG  nextPtr);
    SLOW_PATH_FREE(listPtr);
}


/* =============================================================================
 * list_isEmpty
 * -- Return TRUE if list is empty, else FALSE
 * =============================================================================
 */
bool_t
list_stm::list_isEmpty (list_t* listPtr)
{
    return (listPtr->head.nextPtr == NULL);
}


/* =============================================================================
 * TMlist_isEmpty
 * -- Return TRUE if list is empty, else FALSE
 * =============================================================================
 */
bool_t
list_stm::TMlist_isEmpty (TM_ARGDECL  list_t* listPtr)
{
    void* temp_p = (void*)SLOW_PATH_SHARED_READ_P(listPtr->head.nextPtr);
    return ((temp_p == NULL) ? TRUE : FALSE);
}


/* =============================================================================
 * list_getSize
 * -- Returns the size of the list
 * =============================================================================
 */
long
list_stm::list_getSize (list_t* listPtr)
{
    return listPtr->size;
}


/* =============================================================================
 * TMlist_getSize
 * -- Returns the size of the list
 * =============================================================================
 */
long
list_stm::TMlist_getSize (TM_ARGDECL  list_t* listPtr)
{
    return (long)SLOW_PATH_SHARED_READ(listPtr->size);
}


/* =============================================================================
 * findPrevious
 * =============================================================================
 */
static list_node_t*
findPrevious (list_t* listPtr, void* dataPtr)
{
    list_node_t* prevPtr = &(listPtr->head);
    list_node_t* nodePtr = prevPtr->nextPtr;

    for (; nodePtr != NULL; nodePtr = nodePtr->nextPtr) {
        if (listPtr->compare(nodePtr->dataPtr, dataPtr) >= 0) {
            return prevPtr;
        }
        prevPtr = nodePtr;
    }

    return prevPtr;
}


/* =============================================================================
 * TMfindPrevious
 * =============================================================================
 */
static list_node_t*
TMfindPrevious (TM_ARGDECL  list_t* listPtr, void* dataPtr)
{
    list_node_t* prevPtr = &(listPtr->head);
    list_node_t* nodePtr = (struct list_node*)SLOW_PATH_SHARED_READ_P(prevPtr->nextPtr);
    while(nodePtr != NULL){
        if (listPtr->compare(nodePtr->dataPtr, dataPtr) >= 0) {
            return prevPtr;
        }
        prevPtr = nodePtr;
	nodePtr = SLOW_PATH_SHARED_READ_P(prevPtr->nextPtr);
    }

    return prevPtr;
}


/* =============================================================================
 * list_find
 * -- Returns NULL if not found, else returns pointer to data
 * =============================================================================
 */
void*
list_stm::list_find (list_t* listPtr, void* dataPtr)
{
    list_node_t* nodePtr;
    list_node_t* prevPtr = findPrevious(listPtr, dataPtr);

    nodePtr = prevPtr->nextPtr;

    if ((nodePtr == NULL) ||
        (listPtr->compare(nodePtr->dataPtr, dataPtr) != 0)) {
        return NULL;
    }

    return (nodePtr->dataPtr);
}


/* =============================================================================
 * TMlist_find
 * -- Returns NULL if not found, else returns pointer to data
 * =============================================================================
 */
void*
list_stm::TMlist_find (TM_ARGDECL  list_t* listPtr, void* dataPtr)
{
    list_node_t* nodePtr;
    list_node_t* prevPtr = TMfindPrevious(TM_ARG  listPtr, dataPtr);

    nodePtr = (list_node_t*)SLOW_PATH_SHARED_READ_P(prevPtr->nextPtr);

    if ((nodePtr == NULL) ||
        (listPtr->compare(nodePtr->dataPtr, dataPtr) != 0)) {
        return NULL;
    }

    return (nodePtr->dataPtr);
}


/* =============================================================================
 * list_insert
 * -- Return TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
list_stm::list_insert (list_t* listPtr, void* dataPtr)
{
    list_node_t* prevPtr;
    list_node_t* nodePtr;
    list_node_t* currPtr;

    prevPtr = findPrevious(listPtr, dataPtr);
    currPtr = prevPtr->nextPtr;

#ifdef LIST_NO_DUPLICATES
    if ((currPtr != NULL) &&
        listPtr->compare(currPtr->dataPtr, dataPtr) == 0) {
        return FALSE;
    }
#endif

    nodePtr = allocNode(dataPtr);
    if (nodePtr == NULL) {
        return FALSE;
    }

    nodePtr->nextPtr = currPtr;
    prevPtr->nextPtr = nodePtr;
    listPtr->size++;

    return TRUE;
}


/* =============================================================================
 * Plist_insert
 * -- Return TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
list_stm::Plist_insert (list_t* listPtr, void* dataPtr)
{
    list_node_t* prevPtr;
    list_node_t* nodePtr;
    list_node_t* currPtr;

    prevPtr = findPrevious(listPtr, dataPtr);
    currPtr = prevPtr->nextPtr;

#ifdef LIST_NO_DUPLICATES
    if ((currPtr != NULL) &&
        listPtr->compare(currPtr->dataPtr, dataPtr) == 0) {
        return FALSE;
    }
#endif

    nodePtr = PallocNode(dataPtr);
    if (nodePtr == NULL) {
        return FALSE;
    }

    nodePtr->nextPtr = currPtr;
    prevPtr->nextPtr = nodePtr;
    listPtr->size++;

    return TRUE;
}


/* =============================================================================
 * TMlist_insert
 * -- Return TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
list_stm::TMlist_insert (TM_ARGDECL  list_t* listPtr, void* dataPtr)
{
    list_node_t* prevPtr;
    list_node_t* nodePtr;
    list_node_t* currPtr;

    prevPtr = TMfindPrevious(TM_ARG  listPtr, dataPtr);
    currPtr = (list_node_t*)SLOW_PATH_SHARED_READ_P(prevPtr->nextPtr);

#ifdef LIST_NO_DUPLICATES
    if ((currPtr != NULL) &&
        listPtr->compare(currPtr->dataPtr, dataPtr) == 0) {
        return FALSE;
    }
#endif

    nodePtr = TMallocNode(TM_ARG  dataPtr);
    if (nodePtr == NULL) {
        return FALSE;
    }

    nodePtr->nextPtr = currPtr;
    SLOW_PATH_SHARED_WRITE_P(prevPtr->nextPtr, nodePtr);
    intptr_t temp_l = SLOW_PATH_SHARED_READ(listPtr->size);
    SLOW_PATH_SHARED_WRITE(listPtr->size, (temp_l + 1));

    return TRUE;
}


/* =============================================================================
 * list_remove
 * -- Returns TRUE if successful, else FALSE
 * =============================================================================
 */
bool_t
list_stm::list_remove (list_t* listPtr, void* dataPtr)
{
    list_node_t* prevPtr;
    list_node_t* nodePtr;

    prevPtr = findPrevious(listPtr, dataPtr);

    nodePtr = prevPtr->nextPtr;
    if ((nodePtr != NULL) &&
        (listPtr->compare(nodePtr->dataPtr, dataPtr) == 0))
    {
        prevPtr->nextPtr = nodePtr->nextPtr;
        nodePtr->nextPtr = NULL;
        freeNode(nodePtr);
        listPtr->size--;
        assert(listPtr->size >= 0);
        return TRUE;
    }

    return FALSE;
}


/* =============================================================================
 * Plist_remove
 * -- Returns TRUE if successful, else FALSE
 * =============================================================================
 */
bool_t
list_stm::Plist_remove (list_t* listPtr, void* dataPtr)
{
    list_node_t* prevPtr;
    list_node_t* nodePtr;

    prevPtr = findPrevious(listPtr, dataPtr);

    nodePtr = prevPtr->nextPtr;
    if ((nodePtr != NULL) &&
        (listPtr->compare(nodePtr->dataPtr, dataPtr) == 0))
    {
        prevPtr->nextPtr = nodePtr->nextPtr;
        nodePtr->nextPtr = NULL;
        PfreeNode(nodePtr);
        listPtr->size--;
        assert(listPtr->size >= 0);
        return TRUE;
    }

    return FALSE;
}


/* =============================================================================
 * TMlist_remove
 * -- Returns TRUE if successful, else FALSE
 * =============================================================================
 */
bool_t
list_stm::TMlist_remove (TM_ARGDECL  list_t* listPtr, void* dataPtr)
{
    list_node_t* prevPtr;
    list_node_t* nodePtr;

    prevPtr = TMfindPrevious(TM_ARG  listPtr, dataPtr);

    nodePtr = (list_node_t*)SLOW_PATH_SHARED_READ_P(prevPtr->nextPtr);
    if ((nodePtr != NULL) &&
        (listPtr->compare(nodePtr->dataPtr, dataPtr) == 0))
    {
	list_node_t* temp_p = (list_node_t*)SLOW_PATH_SHARED_READ_P(nodePtr->nextPtr);
        SLOW_PATH_SHARED_WRITE_P(prevPtr->nextPtr, temp_p);
        SLOW_PATH_SHARED_WRITE_P(nodePtr->nextPtr, (struct list_node*)NULL);
        TMfreeNode(TM_ARG  nodePtr);
	intptr_t temp_l = SLOW_PATH_SHARED_READ(listPtr->size);
        SLOW_PATH_SHARED_WRITE(listPtr->size, temp_l - 1);
        assert(listPtr->size >= 0);
        return TRUE;
    }

    return FALSE;
}


/* =============================================================================
 * list_clear
 * -- Removes all elements
 * =============================================================================
 */
void
list_stm::list_clear (list_t* listPtr)
{
    freeList(listPtr->head.nextPtr);
    listPtr->head.nextPtr = NULL;
    listPtr->size = 0;
}


/* =============================================================================
 * Plist_clear
 * -- Removes all elements
 * =============================================================================
 */
void
list_stm::Plist_clear (list_t* listPtr)
{
    PfreeList(listPtr->head.nextPtr);
    listPtr->head.nextPtr = NULL;
    listPtr->size = 0;
}
