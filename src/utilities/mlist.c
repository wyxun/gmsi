#include <stddef.h>
#include "mlist.h"

void mlist_Init(mlist_t * const ptList)
{
    /* The list structure contains a list item which is used to mark the
     * end of the list. To initialise the list the list end is inserted
     * as the only list entry. */
    ptList->ptIndex = (mlist_item_t *) &( ptList->tListEnd );

    /* The list end value is the highest possible value in the list to
     * ensure it remains at the end of the list. */
    ptList->tListEnd.wItemValue = 0xFFFFFFFFUL;

    /* The list end next and previous pointers point to itself so we know
     * when the list is empty. */
    ptList->tListEnd.ptNext = (mlist_item_t *) &( ptList->tListEnd );
    ptList->tListEnd.ptPrevious = (mlist_item_t *) &( ptList->tListEnd );

    ptList->wNumberOfItems = 0;
}

void mlist_ItemInit(mlist_item_t * const ptItem)
{
    /* Make sure the list item is not recorded as being on a list. */
    ptItem->ptContainer = NULL;
}

void mlist_InsertEnd(mlist_t * const ptList, mlist_item_t * const ptNewListItem)
{
    mlist_item_t * const ptIndex = ptList->ptIndex;

    /* Insert a new list item into ptList, but rather than sort the list,
     * makes the new list item the last item to be removed by a call to
     * MLIST_GET_OWNER_OF_NEXT_ENTRY. */
    ptNewListItem->ptNext = ptIndex;
    ptNewListItem->ptPrevious = ptIndex->ptPrevious;

    ptIndex->ptPrevious->ptNext = ptNewListItem;
    ptIndex->ptPrevious = ptNewListItem;

    /* Remember which list the item is in. */
    ptNewListItem->ptContainer = ptList;

    (ptList->wNumberOfItems)++;
}

void mlist_Insert(mlist_t * const ptList, mlist_item_t * const ptNewListItem)
{
    mlist_item_t *ptIterator;
    const uint32_t wValueOfInsertion = ptNewListItem->wItemValue;

    /* Insert the new list item into the list, sorted in descending order.
     * If the value is 0xFFFFFFFFUL, place it at the end (before marker). */
    if(wValueOfInsertion == 0xFFFFFFFFUL)
    {
        ptIterator = ptList->tListEnd.ptPrevious;
    }
    else
    {
        /* Otherwise, traverse the list to find the correct position. */
        for (ptIterator = (mlist_item_t *)&(ptList->tListEnd); 
             ptIterator->ptNext->wItemValue <= wValueOfInsertion; 
             ptIterator = ptIterator->ptNext)
        {
            /* There is nothing to do here, just iterating. */
        }
    }

    ptNewListItem->ptNext = ptIterator->ptNext;
    ptNewListItem->ptNext->ptPrevious = ptNewListItem;
    ptNewListItem->ptPrevious = ptIterator;
    ptIterator->ptNext = ptNewListItem;

    ptNewListItem->ptContainer = ptList;

    (ptList->wNumberOfItems)++;
}

uint32_t mlist_Remove(mlist_item_t * const ptItemToRemove)
{
    mlist_t * const ptList = ptItemToRemove->ptContainer;

    ptItemToRemove->ptNext->ptPrevious = ptItemToRemove->ptPrevious;
    ptItemToRemove->ptPrevious->ptNext = ptItemToRemove->ptNext;

    /* Make sure the index is left pointing to a valid item. */
    if(ptList->ptIndex == ptItemToRemove)
    {
        ptList->ptIndex = ptItemToRemove->ptPrevious;
    }

    ptItemToRemove->ptContainer = NULL;
    (ptList->wNumberOfItems)--;

    return ptList->wNumberOfItems;
}
