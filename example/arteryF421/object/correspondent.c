#include "correspondent.h"
#include "userconfig.h"
#include <string.h>
#include "user_protocol.h"

int correspondent_Run(uintptr_t wObjectAddr);
int correspondent_Clock(uintptr_t wObjectAddr);
static uint8_t s_chCorrespondentGetData[100];

// define cmd
#define CMD_READ_IMU            0x01
#define CMD_READ_NFC            0x02
#define CMD_CALIBRATION_IMU     0x03
#define CMD_READ_LIQ_ALLOWANCE  0x04
#define CMD_SPRAY_WORK          0x05
#define CMD_SPRAY_RESET         0x06

// gmsi
static gmsi_base_t s_tCorrespondentBase;
gmsi_base_cfg_t s_tCorrespondentBaseCfg = {
    .wId = CORRESPONDENT,
    .wParent = 0,
    .FcnInterface = {
        .Clock = correspondent_Clock,
        .Run = correspondent_Run,
    },
};

GMSI_MSG_ITEM_DECLARE(CORRESPONDENT, CaptureingData, 40);
gcoroutine_handle_t tGcoroutineCorrespondentHandle = {
    .bIsRunning = false,
    .pfcn = NULL,
};


// Correspondent response callback function 
void correspondent_response_callback(uint8_t chCmd, const uint8_t *pchData, uint8_t chDataLen)
{
    uint8_t chHandleArray[30];
    uint16_t hwFrameLen;
    // 处理应答
    switch(chCmd)
    {
        // 获取坐标
        case CMD_READ_IMU:
            // 获取坐标失败
            chHandleArray[0] = 0x00;
            pack_protocol_response_frame(chCmd, chHandleArray, 1);
        break;
        // 读标签
        case CMD_READ_NFC:
            gbase_EventPost(RFID, twoInOneHandle_Event_ReadNfcData);
        break;
        case CMD_CALIBRATION_IMU:
            break;
        // 读取剩余容量，单位时间，精度0.01s，数值÷100为s
        case CMD_READ_LIQ_ALLOWANCE:
            break;
        // 喷雾工作
        case CMD_SPRAY_WORK:
            break;
        // 复位喷雾推杆
        case CMD_SPRAY_RESET:
            break;
        default:
            break;
    }
}

/**
 * Function: correspondent_EventHandle
 * ----------------------------
 * This function handles events for the correspondent object.
 *
 * Parameters: 
 * ptThis: The address of the correspondent object.
 * wEvent: The event to handle.
 *
 * Returns: 
 * None
 */
void correspondent_EventHandle(correspondent_t *ptThis, uint32_t wEvent)
{

}

/**
 * Function: correspondent_Run
 * ----------------------------
 * This function runs the correspondent object. It handles events for the object, reads data from a file descriptor, 
 * checks the data's source and handles it accordingly. If the data length is not zero, it writes the data back to the file descriptor.
 *
 * Parameters: 
 * wObjectAddr: The address of the correspondent object.
 *
 * Returns: 
 * A status code indicating the result of the function. The possible return values depend on the 
 * implementation of the GMSI_SUCCESS constant.
 */
int correspondent_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint16_t hwLength = 0;

    // get ptr
    correspondent_t *ptThis = (correspondent_t *)wObjectAddr;
    
    // handle event
    uint32_t wEvent = gbase_EventPend(ptThis->ptBase);
    correspondent_EventHandle(ptThis, wEvent);
    //uint8_t* pchData = GMSI_MSG_ITEM_GET_BUFFER(CorrespondentFrameData);
    // handle uart
    hwLength = ptThis->Read(ptThis->wFd, s_chCorrespondentGetData);
    if(hwLength)
    {
        // test echo
        //ptThis->Write(ptThis->wFd, s_chCorrespondentGetData, hwLength);
        protocol_parse_and_handle(s_chCorrespondentGetData, hwLength);
        
        // 测试电机
        if(s_chCorrespondentGetData[0] == 0x0b)
            gbase_EventPost(STEP4988, twoInOneHandle_Event_StartStepMotor);
        else if(s_chCorrespondentGetData[0] == 0x0c)
            gbase_EventPost(STEP4988, twoInOneHandle_Event_StopStepMotor);
        else if(s_chCorrespondentGetData[0] == 0x0d)
            gbase_EventPost(STEP4988, twoInOneHandle_Event_MotorDirCCW);
        else if(s_chCorrespondentGetData[0] == 0x0e)
            gbase_EventPost(STEP4988, twoInOneHandle_Event_MotorDirCW);
    }
    
    hwLength = gbase_MessagePendFromRing(ptThis->ptBase, s_chCorrespondentGetData, 100);
    if(hwLength == 24)
    {
        //ptThis->Write(ptThis->wFd,s_chCorrespondentGetData, hwLength);
        pack_protocol_response_frame(0x02, s_chCorrespondentGetData, 24);
    }
    else if(hwLength == 1)
    {
        s_chCorrespondentGetData[0] = 0;
        pack_protocol_response_frame(0x02, s_chCorrespondentGetData, 1);
    }
    
    return wRet;
}

/**
 * Function: correspondent_Clock
 * ----------------------------
 * This function is the clock function for the correspondent object. It decrements the poll counter and tests for a timeout.
 *
 * Parameters: 
 * wObjectAddr: The address of the correspondent object.
 *
 * Returns: 
 * A status code indicating the result of the function. The possible return values depend on the 
 * implementation of the GMSI_SUCCESS constant.
 */
int correspondent_Clock(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    static uint16_t s_hwTimeout = 0;
    correspondent_t *ptThis = (correspondent_t *)wObjectAddr;
    
    return wRet;
}

/**
 * Function: correspondent_Init
 * ----------------------------
 * This function initializes the correspondent object. It sets the object's base, parent, read and write functions.
 *
 * Parameters: 
 * wObjectAddr: The address of the correspondent object.
 * wObjectCfgAddr: The address of the correspondent configuration object.
 *
 * Returns: 
 * A status code indicating the result of the function. The possible return values depend on the 
 * implementation of the GMSI_SUCCESS constant.
 */
int correspondent_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;

    correspondent_t *ptThis = (correspondent_t *)wObjectAddr;
    correspondent_cfg_t *ptCfg = (correspondent_cfg_t *)wObjectCfgAddr;
    
    protocol_register_response_callback(correspondent_response_callback);
    
    ptThis->ptBase = &s_tCorrespondentBase;

    if (ptThis->ptBase == NULL) {
        wRet = GMSI_EAGAIN;
    } else {
        s_tCorrespondentBaseCfg.wParent = wObjectAddr;
        if(ptCfg->pchRingBuffer != NULL && ptCfg->hwRingSize != 0) {
            // Initialize the ring buffer in the example object
            ptThis->ptBase->tRingBuffer.buffer = ptCfg->pchRingBuffer;
            ptThis->ptBase->tRingBuffer.hwBufferSize = ptCfg->hwRingSize;
            ptThis->ptBase->tRingBuffer.hwWriteIndex = 0;
            ptThis->ptBase->tRingBuffer.hwReadIndex = 0;
        }
        wRet = gbase_Init(ptThis->ptBase, &s_tCorrespondentBaseCfg);
    }   
    
    if(NULL != ptCfg->Read)
        ptThis->Read = ptCfg->Read;
    else
        wRet = GMSI_EAGAIN;
    
    if(NULL != ptCfg->Write)
        ptThis->Write = ptCfg->Write;
    else
        wRet = GMSI_EAGAIN;
    
    ptThis->wFd = ptCfg->wFd;
    
    protocol_register_response_func(ptThis->Write);
    return wRet;
}



