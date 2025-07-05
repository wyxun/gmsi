#include "global_define.h"
#include "utilities/list.h"
#include "utilities/util_debug.h"
#include "gbase.h"
#include <string.h>

#ifdef LINUX_POSIX
#include <stdio.h>
#include <assert.h>
#endif

static struct xLIST tListObject;

/**
 * Function: gbase_Init
 * ----------------------------
 * This function initializes a gmsi_base_t structure. It sets the structure's ID and event, 
 * initializes its list item, and inserts the item into a list. If the parent of the configuration 
 * structure is not zero, it sets the parent of the base structure; otherwise, it returns GMSI_EAGAIN.
 *
 * Parameters: 
 * ptBase: A pointer to the gmsi_base_t structure to initialize.
 * ptCfg: A pointer to the configuration structure for the base structure.
 *
 * Returns: 
 * A status code indicating the result of the function. GMSI_SUCCESS if the function succeeds, 
 * GMSI_EINVAL if the parent of the configuration structure is zero.
 */
int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg)
{
    int wRet = GMSI_SUCCESS;
    static uint8_t chInitCount = 0;

    // Check for null pointers
    if (ptBase == NULL || ptCfg == NULL) {
        return GMSI_EINVAL;
    }
    // Initialise list object on first call
    if(!chInitCount)
        vListInitialise(&tListObject);
    chInitCount++;

    // Set base ID and clear event
    ptBase->wId = ptCfg->wId;
    ptBase->wEvent = 0;

    // Initialise list item and set its value and owner
    vListInitialiseItem(&ptBase->tListItem);
    ptBase->tListItem.xItemValue = ptBase->wId;
    ptBase->tListItem.pvOwner = ptBase;

    // Initialise message list
    vListInitialise(&ptBase->tListMessage);
    // Insert item into list
    vListInsert(&tListObject, &ptBase->tListItem);

    // Set parent and function interface
    if(ptCfg->wParent)
        ptBase->wParent = ptCfg->wParent;
    else
        wRet = GMSI_EAGAIN;
    ptBase->pFcnInterface = &ptCfg->FcnInterface;
    return wRet;
}

/**
 * Function: gbase_EventPost
 * ----------------------------
 * This function posts an event to a base object with a specific ID. It traverses a list of base objects, 
 * finds the one with the given ID, and updates its event. If no base object with the given ID is found, 
 * it returns an error code.
 *
 * Parameters: 
 * wId: The ID of the base object to post the event to.
 * wEvent: The event to post.
 *
 * Returns: 
 * A status code indicating the result of the function. GMSI_SUCCESS if the function succeeds, 
 * GMSI_ENODEV if no base object with the given ID is found.
 */
int gbase_EventPost(uint32_t wId, uint32_t wEvent)
{
    int wRet = GMSI_SUCCESS;
    uint8_t chErgodicTime = 1;
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;;
    gmsi_base_t *ptBaseDes;

    // Check for valid input
    if (wId == 0 || wEvent == 0) {
        return GMSI_EINVAL;
    }

    // Find the list item with the given ID
    for (; ptListItemDes != &tListObject.xListEnd; ptListItemDes = ptListItemDes->pxPrevious, chErgodicTime++) {
        if(ptListItemDes->xItemValue == wId)
            break;
    }

    // If the item was found, update its event
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {
        ptBaseDes = ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptBaseDes);

        ptBaseDes->wEvent |= wEvent;
    }
    else
    {
        // If the item was not found, return an error
        wRet = GMSI_ENODEV;
    }
    
    return wRet;
}

/**
 * Function: gbase_EventPend
 * ----------------------------
 * This function retrieves and clears the event of a gmsi_base_t structure. If the event is not zero, 
 * it clears the event and returns the original event.
 *
 * Parameters: 
 * ptBase: A pointer to the gmsi_base_t structure to retrieve the event from.
 *
 * Returns: 
 * The original event of the gmsi_base_t structure, or zero if the event was zero or ptBase is NULL.
 */
uint32_t gbase_EventPend(gmsi_base_t *ptBase)
{
    // Check for null pointer
    if (ptBase == NULL) {
        return 0;
    }

    uint32_t wEvent = ptBase->wEvent;
    if(0 != wEvent)
    {
        ptBase->wEvent = 0;
    }
    return wEvent;
}

/**
 * Function: gbase_MessagePost
 * ----------------------------
 * This function posts a message to a base object with a specific ID. It traverses a list of base objects, 
 * finds the one with the given ID, and updates its message and length. If no base object with the given ID 
 * is found, it returns an error code.
 *
 * Parameters: 
 * wId: The ID of the base object to post the message to.
 * ptMsgItem: A pointer to the message_item_t structure containing the message and its length.
 *
 * Returns: 
 * A status code indicating the result of the function. GMSI_SUCCESS if the function succeeds, 
 * GMSI_EINVAL if the ptMsgItem pointer is NULL, or GMSI_ENODEV if no base object with the given ID is found.
 */
int gbase_MessagePost(uint32_t wId, message_item_t *ptMsgItem)
{
    // Check for valid input
    if (wId == 0 || ptMsgItem == NULL) {
        return GMSI_EINVAL;
    }

    int wRet = 0;
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;
    uint8_t chErgodicTime = 1;
    gmsi_base_t *ptBaseDes;

    // Find the list item with the given ID
    for (uint8_t chErgodicTime = 1;                                         \
            ptListItemDes != &tListObject.xListEnd;                         \
            ptListItemDes = ptListItemDes->pxPrevious, chErgodicTime++) {
        if(ptListItemDes->xItemValue == wId)
            break;
    }

    // If the item was found, update its message and length, and set a transition event
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {
        ptBaseDes = ptListItemDes->pvOwner;
        ptMsgItem->tListItem.pvOwner = ptMsgItem;
        GMSI_ASSERT(NULL != ptBaseDes);
        // if message length is not zero, insert message into list
        if(ptMsgItem->hwLength > 0)
        {
            vListInsert(&ptBaseDes->tListMessage, &ptMsgItem->tListItem);
        }
        
        ptBaseDes->wEvent |= Gmsi_Event_Transition;
    }
    else
    {
        // If the item was not found, return an error
        wRet = GMSI_ENODEV;
    }
    
    return wRet;
}

/**
 * Function: gbase_MessagePend
 * ----------------------------
 * This function retrieves and removes a message from a gmsi_base_t structure. It copies the message and its 
 * length to the message_t structure pointed to by ptMsg. If the message is successfully retrieved, the function 
 * returns the length of the message; otherwise, it returns an error code.
 *
 * Parameters: 
 * ptBase: A pointer to the gmsi_base_t structure to retrieve the message from.
 * ptMsg: A pointer to the message_t structure to copy the message and its length to.
 *
 * Returns: 
 * The length of the message, or an error code if the message was not successfully retrieved.
 */
int gbase_MessagePend(gmsi_base_t *ptBase, message_t *ptMsg)
{
    // Check for null pointer
    if (ptBase == NULL) {
        return GMSI_EINVAL;
    }

    int wRet = GMSI_SUCCESS;
    struct xLIST_ITEM *ptListItemDes = ptBase->tListMessage.xListEnd.pxPrevious;
    message_t *ptMsgDes;

    // Find the list item with the given ID
    if(ptListItemDes != &ptBase->tListMessage.xListEnd)
    {
        ptMsgDes = (message_t *)ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptMsgDes);

        // copy message and length
        ptMsg->hwLength = ptMsgDes->hwLength;
        memcpy(ptMsg->pchMessage, ptMsgDes->pchMessage, ptMsgDes->hwLength);

        // remove message from list
        uxListRemove(ptListItemDes);

        wRet = ptMsg->hwLength;
    }
    else
    {
        // If the item was not found, return an error
        wRet = GMSI_ENODEV;
    }
    return wRet;
}

/**
 * Function: gbase_MessagePostToRing
 * ---------------------------------
 * This function posts a message to a ring buffer of a base object with a specific ID. It traverses a list 
 * of base objects, finds the one with the given ID, and updates its ring buffer with the message. If no base 
 * object with the given ID is found, it returns an error code.
 *
 * Parameters: 
 * wId: The ID of the base object to post the message to.
 * pchMsgBuffer: A pointer to the message buffer containing the message to post.
 * hwLength: The length of the message.
 *
 * Returns: 
 * A status code indicating the result of the function. GMSI_SUCCESS if the function succeeds, 
 * GMSI_EINVAL if any input parameter is invalid, or GMSI_ENODEV if no base object with the given ID is found.
 */
int gbase_MessagePostToRing(uint32_t wId, uint8_t *pchMsgBuffer, uint16_t hwLength)
{
    // Check for valid input
    if (wId == 0 || pchMsgBuffer == NULL || hwLength == 0) {
        return GMSI_EINVAL;
    }

    int wRet = 0;
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;
    uint8_t chErgodicTime = 1;
    gmsi_base_t *ptBaseDes;

    // Find the list item with the given ID
    for (uint8_t chErgodicTime = 1;                                         \
            ptListItemDes != &tListObject.xListEnd;                         \
            ptListItemDes = ptListItemDes->pxPrevious, chErgodicTime++) {
        if(ptListItemDes->xItemValue == wId)
            break;
    }

    // If the item was found, update its message and length, and set a transition event
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {
        ptBaseDes = ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptBaseDes);
        object_ring_buffer_t *pRing = &ptBaseDes->tRingBuffer;
        if (pRing->buffer && pRing->hwBufferSize > 0) {
            for (uint16_t i = 0; i < hwLength; ++i) {
                pRing->buffer[pRing->hwWriteIndex] = pchMsgBuffer[i];
                pRing->hwWriteIndex = (pRing->hwWriteIndex + 1) % pRing->hwBufferSize;
                // If the write index catches up to the read index, increment the read index
                if (pRing->hwWriteIndex == pRing->hwReadIndex) {
                    pRing->hwReadIndex = (pRing->hwReadIndex + 1) % pRing->hwBufferSize;
                }
            }
        }
        ptBaseDes->wEvent |= Gmsi_Event_Transition;
    }
    else
    {
        // If the item was not found, return an error
        wRet = GMSI_ENODEV;
    }

    return wRet;
}

int gbase_MessagePendFromRing(gmsi_base_t *ptBase, uint8_t *pchMsgBuffer, uint16_t hwMaxSize)
{
    // Check for null pointer
    if (ptBase == NULL || pchMsgBuffer == NULL || hwMaxSize == 0) {
        return GMSI_EINVAL;
    }

    int wRet = 0;
    object_ring_buffer_t *ptRing = &ptBase->tRingBuffer;

    uint16_t hwLength = 0;
    while (ptRing->hwReadIndex != ptRing->hwWriteIndex && hwLength < hwMaxSize) {
        pchMsgBuffer[hwLength++] = ptRing->buffer[ptRing->hwReadIndex];
        ptRing->hwReadIndex = (ptRing->hwReadIndex + 1) % ptRing->hwBufferSize;
    }
    wRet = hwLength; // Return the number of bytes read, 0 if no data
    return wRet;
}
/**
 * Function: gbase_DebugListBase
 * ----------------------------
 * This function prints the IDs of all objects in the global list object.
 *
 * Parameters: 
 * None
 *
 * Returns: 
 * None
 */
void gbase_DebugListBase(void)
{
    LOG_OUT("List all object:\n");

    // Traverse the list and print each item's ID
    for (struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious; ptListItemDes != &tListObject.xListEnd; ptListItemDes = ptListItemDes->pxPrevious) {
        LOG_OUT("    item id:");
        LOG_OUT((uint32_t)ptListItemDes->xItemValue);
        LOG_OUT("\n");
    }
}

/**
 * Function: gbase_GetBaseList
 * ----------------------------
 * This function returns a reference to the global list object.
 *
 * Parameters: 
 * None
 *
 * Returns: 
 * A pointer to the global list object.
 */
struct xLIST* gbase_GetBaseList(void)
{
    // Return a reference to the global list object
    return &tListObject;
}