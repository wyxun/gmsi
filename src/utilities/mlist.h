#ifndef __MODUS_LIST_H__
#define __MODUS_LIST_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * MODUS Doubly Linked List Implementation
 * 
 * A clean, lightweight doubly linked list implementation for the MODUS framework.
 * It is sorted by item value in descending order by default.
 * Includes a marker element (tListEnd) to eliminate edge cases 
 * during insertion/removal.
 */

/* Definition of a list item. */
struct mlist;
typedef struct mlist_item
{
    uint32_t wItemValue;           /*< Value being listed. Used to sort. */
    struct mlist_item *ptNext;     /*< Pointer to next item. */
    struct mlist_item *ptPrevious; /*< Pointer to previous item. */
    void *pvOwner;                 /*< Pointer to owner object. */
    struct mlist *ptContainer;     /*< Pointer to containing list. */
} mlist_item_t;

/* Definition of a list. */
typedef struct mlist
{
    uint32_t wNumberOfItems;       /*< Number of items in list. */
    mlist_item_t *ptIndex;         /*< Index for next entry walk. */
    mlist_item_t tListEnd;         /*< End marker (highest value). */
} mlist_t;

/* Access macros */
#define MLIST_SET_ITEM_OWNER(ptListItem, ptOwner)                               \
    ((ptListItem)->pvOwner = (void *)(ptOwner))
#define MLIST_GET_ITEM_OWNER(ptListItem)             ((ptListItem)->pvOwner)
#define MLIST_SET_ITEM_VALUE(ptListItem, wValue)                                \
    ((ptListItem)->wItemValue = (wValue))
#define MLIST_GET_ITEM_VALUE(ptListItem)             ((ptListItem)->wItemValue)
#define MLIST_GET_HEAD_ENTRY(ptList)                 (((ptList)->tListEnd).ptNext)
#define MLIST_GET_END_MARKER(ptList)                 (&( (ptList)->tListEnd ))
#define MLIST_IS_EMPTY(ptList)                                                  \
    (((ptList)->wNumberOfItems == 0) ? true : false)
#define MLIST_CURRENT_LIST_LENGTH(ptList)            ((ptList)->wNumberOfItems)
#define MLIST_ITEM_CONTAINER(ptListItem)             ((ptListItem)->ptContainer)

/*
 * Access function to obtain the owner of the next entry in a list.
 * Automatically skips the end marker.
 */
#define MLIST_GET_OWNER_OF_NEXT_ENTRY(ptOwner, ptList)                          \
    do {                                                                        \
        mlist_t * const ptConstList = (ptList);                                 \
        ptConstList->ptIndex = ptConstList->ptIndex->ptNext;                    \
        if((void *)ptConstList->ptIndex == (void *)&ptConstList->tListEnd) {    \
            ptConstList->ptIndex = ptConstList->ptIndex->ptNext;                \
        }                                                                       \
        (ptOwner) = ptConstList->ptIndex->pvOwner;                              \
    } while(0)

/*
 * Access function to obtain the owner of the first entry in a list.
 */
#define MLIST_GET_OWNER_OF_HEAD_ENTRY(ptList)                                   \
    ( (&((ptList)->tListEnd))->ptNext->pvOwner )

/* API Functions */
void mlist_Init(mlist_t * const ptList);
void mlist_ItemInit(mlist_item_t * const ptItem);
void mlist_Insert(mlist_t * const ptList, mlist_item_t * const ptNewListItem);
void mlist_InsertEnd(mlist_t * const ptList, mlist_item_t * const ptNewListItem);
uint32_t mlist_Remove(mlist_item_t * const ptItemToRemove);

#ifdef __cplusplus
}
#endif

#endif /* __MODUS_LIST_H__ */
