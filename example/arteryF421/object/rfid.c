#include "rfid.h"
#include "userconfig.h"
#include "at32f421_bsp.h"
#include "perf_counter.h"

int rfid_Clock(uintptr_t wObjectAddr);
int rfid_Run(uintptr_t wObjectAddr);

extern rfid_io_t c_tRFIDIO;
// Define a global rfid base of type gmsi_base_t
static gmsi_base_t s_tRfidBase;

// Define and initialize a global rfid base configuration of type gmsi_base_cfg_t
gmsi_base_cfg_t s_tRfidBaseCfg = {
    .wId = RFID,                 // Set the ID to RFID
    .wParent = 0,                // Set the parent to 0
    .FcnInterface = {
        .Clock = rfid_Clock,     // Set the Clock function to rfid_Clock
        .Run = rfid_Run,         // Set the Run function to rfid_Run
    },
};

gcoroutine_handle_t tGcoroutineRfidHandle = {
    .bIsRunning = false,
    .pfcn = NULL,
};

/**
 * Function: rfid_gcoroutine
 * ----------------------------
 * This function is a coroutine that handles different states of the rfid object. It receives 
 * a parameter, converts it to an rfid object, and uses a switch statement to handle different 
 * states. This function returns fsm_rt_on_going if it is still running, or fsm_rt_cpl if it has 
 * completed its task. Once the coroutine has completed its task, it will be removed from the 
 * coroutine list.
 *
 * Parameters: 
 * pvParam: The parameter to be converted to an rfid object.
 *
 * Returns: 
 * fsm_rt_on_going if the function is still running, or fsm_rt_cpl if the function has completed 
 * its task.
 */
fsm_rt_t rfid_gcoroutine(void *pvParam)
{
    static uint8_t s_eState = 0;
    fsm_rt_t tFsm = fsm_rt_on_going;
    rfid_t *ptThis = (rfid_t *)pvParam;

    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("Error: ptThis is NULL.\n");
        return fsm_rt_err;
    }

    switch(s_eState)
    {
        case 0:
            GLOG_PRINTF("get rfid event");
            s_eState++;
            break;
        case 1:
            GLOG_PRINTF("finish get rfid event handle");
            fsm_cpl();
            break;
        default:
            fsm_cpl();
        break;
    }
    fsm_on_going(); 
}

// 新增校验和计算函数
static uint8_t rfid_CalculateBCC(uint8_t *pchData, uint8_t chLength)
{
    uint8_t chBCC = 0;
    for(uint8_t i = 0; i < chLength; i++) {
        chBCC ^= pchData[i];
    }
    return ~chBCC;
}

// 通用指令发送函数
static int rfid_SendCommand(rfid_t *ptThis, uint8_t chCmd, uint8_t *pchData, uint8_t chDataLen)
{
    uint8_t chFrame[32] = {
        0x20,   // STX
        ptThis->chAddr, // ADDR
        chCmd,  // Command
        chDataLen // Data Length
    };
    
    // 拷贝数据段
    if(chDataLen > 0 && pchData != NULL) {
        // 手动实现字节拷贝
        for(uint8_t i = 0; i < chDataLen; i++) {
            chFrame[4 + i] = pchData[i];
        }
    }
    
    // 计算BCC校验（从ADDR开始到数据结束）
    uint8_t chBCC = rfid_CalculateBCC(&chFrame[1], 3 + chDataLen);
    
    // 添加校验和与结束符
    chFrame[4 + chDataLen] = chBCC;
    chFrame[5 + chDataLen] = 0x03; // ETX
    
    // 通过配置的串口接口发送
    if(ptThis->Write(ptThis->wFd, chFrame, 6 + chDataLen) == 0) {
        return GMSI_EFAIL;
    }
    return GMSI_SUCCESS;
}

/**
 * Function: rfid_EventHandle
 * ----------------------------
 * This function handles the events for the rfid object. It checks for specific events and 
 * performs the corresponding actions. This function does not return a value, as its result is 
 * reflected in the state of the rfid object it operates on.
 *
 * Parameters: 
 * ptThis: The pointer to the rfid object.
 * wEvent: The events to be handled.
 *
 * Returns: 
 * None
 */
static void rfid_EventHandle(rfid_t *ptThis, uint32_t wEvent)
{
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return;
    }

#if 1 // 启用事件处理
    if(wEvent & twoInOneHandle_Event_ReadNfcData) 
    {
        // 有数据
        if(ptThis->tNfcMessage.chNfcType == NFC_TYPE_NTAG213)
        {
            gbase_MessagePostToRing(CORRESPONDENT, (uint8_t *)&ptThis->tNfcMessage, 24);
        }
        else
        {
            gbase_MessagePostToRing(CORRESPONDENT, (uint8_t *)&ptThis->tNfcMessage.chNfcType, 1);
        }
    }
#endif
}

// 新增配置函数示例
int rfid_Configure(rfid_t *ptThis, uint8_t chSettingType, uint8_t chValue)
{
    uint8_t chData[4] = {0};
    switch(chSettingType) {
    case 0x2C: // 功能配置
        chData[0] = 0x00; // 主动输出控制
        chData[1] = 0x00; // 离开指令控制 
        chData[2] = chValue; // 波特率设置
        chData[3] = 0x00; // 蜂鸣器控制
        return rfid_SendCommand(ptThis, 0x2C, chData, 4);
        
    case 0x2B: // 地址修改
        return rfid_SendCommand(ptThis, 0x2B, &chValue, 1);
        
    default:
        return GMSI_EINVAL;
    }
}

// 自定义回调函数
void rfid_RecvHandle(rfid_t *ptThis, uint8_t chCmd, uint8_t *pchData, uint8_t chLength)
{
    uint8_t chData[7] = {0};
    switch(chCmd)
    {
        case 0x27:
            // 自定义卡处理逻辑
            break;
        case 0x00:
            // NTAG213 UUID
            if(pchData[0] == 0x44 && pchData[1] == 0x00) 
            {
                for(uint8_t i = 0; i < 7; i++) {
                    ptThis->tNfcMessage.chNfcUUID[i] = pchData[i+2];
                    GVAL_PRINTF(pchData[i+2]);
                }
                // 发送读取用户区数据请求
                chData[0] = 0x00; // 读取0-3用户区
                rfid_SendCommand(ptThis, 0x52, chData, 1);
                ptThis->tNfcMessage.chNfcType = NFC_TYPE_UNKNOWN;
            }
        // 读卡成功
        case 0x52:
            if(chLength == 0x11)
            {
                for(uint8_t i = 0; i < 16; i++) {
                    ptThis->tNfcMessage.chData[i+pchData[0]*4] = pchData[i+1];
                }
                ptThis->tNfcMessage.chNfcType = NFC_TYPE_NTAG213; // 设置NFC类型
                // 启动读取下四个块数据
//                else if(pchData[0] < 4)
//                {
//                    chData[0] = 0x03; // 读取4-7用户区
//                    rfid_SendCommand(ptThis, 0x52, chData, 1);
//                }
            }
            else
            {
                ptThis->tNfcMessage.chNfcType = NFC_TYPE_UNKNOWN;
            }
            break;
        break;
    }
}

/**
 * Function: rfid_Run
 * ----------------------------
 * This function is mounted in the GMSI run function list and is called within a while(1) loop. 
 * It receives events and messages from other objects. When a corresponding event is triggered, 
 * it executes the appropriate coroutine. The function gets the rfid object from the given 
 * address, retrieves the events for the object, and handles these events. This function returns 
 * GMSI_SUCCESS if it runs successfully.
 *
 * Parameters: 
 * wObjectAddr: The address of the rfid object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int rfid_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
 
    // Get the rfid object from the given address
    rfid_t *ptThis = (rfid_t *)wObjectAddr;

    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return GMSI_EFAIL;
    }

    // Get the events for the rfid object
    wEvent = gbase_EventPend(ptThis->ptBase);
    // If there are any events, handle them
    if(wEvent)
        rfid_EventHandle(ptThis, wEvent);
    
    uint8_t chBuffer[32];
    uint16_t hwLength = ptThis->Read(ptThis->wFd, chBuffer);
    // 验证数据帧格式
    if(hwLength >= 6 && 
        chBuffer[0] == 0x20 && 
        chBuffer[hwLength-1] == 0x03)
    {
        // 提取数据并处理
        uint8_t chCmd = chBuffer[2];
        uint8_t chDataLen = chBuffer[3];
        
        rfid_RecvHandle(ptThis, chCmd, &chBuffer[4], chDataLen);
    }
    // Logic or state machine programs

    return wRet;
}

/**
 * Function: rfid_Clock
 * ----------------------------
 * This function is called in a 1ms interrupt. It gets the rfid object from the given address 
 * and performs operations on it. This function returns GMSI_SUCCESS if it runs successfully.
 *
 * Parameters: 
 * wObjectAddr: The address of the rfid object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int rfid_Clock(uintptr_t wObjectAddr)
{
    static uint8_t chCardReadStatus = 0;
    static uint8_t chCardStatusChangeCount = 0;
    // Get the rfid object from the given address
    rfid_t *ptThis = (rfid_t *)wObjectAddr;
    static uint16_t s_hwTestDelay = 1000;
    int wRet = GMSI_SUCCESS;
    
    // 卡状态检测
    if(chCardReadStatus != RFID_CARDIN_READ)
    {
        chCardStatusChangeCount++;
        if(chCardStatusChangeCount > 5)
        {
            chCardReadStatus = RFID_CARDIN_READ;
            if(!chCardReadStatus)   // 下降沿拔卡
            {
                ptThis->tNfcMessage.chNfcType = NFC_TYPE_UNKNOWN;
            }
            chCardStatusChangeCount = 0;
        }
    }

    // Perform operations on ptThis

    return wRet;
}

/**
 * Function: rfid_Init
 * ----------------------------
 * This function initializes an rfid object. It converts the given addresses to pointers, 
 * checks if the pointers are not NULL, copies the configuration members to the object, initializes 
 * the hardware, and registers the object in the GMSI list.
 *
 * Parameters: 
 * wObjectAddr: The address of the rfid object.
 * wObjectCfgAddr: The address of the rfid configuration object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully, GMSI_EAGAIN if the object cannot be registered 
 * in the GMSI list, or GMSI_FAILURE if any of the pointers is NULL.
 */
int rfid_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    // Convert the given addresses to pointers
    rfid_t *ptThis = (rfid_t *)wObjectAddr;
    rfid_cfg_t *ptCfg = (rfid_cfg_t *)wObjectCfgAddr;

    // Check if the pointers are not NULL
    if (ptThis == NULL || ptCfg == NULL) {
        GLOG_PRINTF("Error: ptThis or ptCfg is NULL.");
        return GMSI_EFAIL;
    }

    /* Copy the configuration members to the object */
    ptThis->Read = ptCfg->Read;
    ptThis->Write = ptCfg->Write;
    ptThis->wFd = ptCfg->wFd;
    /* Initialize the hardware */

    // Register the object in the GMSI list
    ptThis->ptBase = &s_tRfidBase;
    if (ptThis->ptBase == NULL) {
        return GMSI_EAGAIN;
    } else {
        s_tRfidBaseCfg.wParent = wObjectAddr;
        return gbase_Init(ptThis->ptBase, &s_tRfidBaseCfg);
    }
    ptThis->tNfcMessage.chNfcType = NFC_TYPE_UNKNOWN;
    RFID_RESET;
    delay_ms(100);
    // test
    
}



