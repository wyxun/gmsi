### GMSI���

**Generic Mcu Software Infrastructure**

![framework](.assets/framework.jpg)

#### �ļ��ֲ����

```c
// gmsi���ļ����Խ�main��������ģ��ʵ��
gmsi.c/h
// gmsiͨ�ýӿ�ʵ��
gmsi_common.c/h
// gmsiģ��������
gmsi_test.c/h
// gmsi������ڣ����û��ü�����ʹ��
gmsi_configh.h
// gmsi��ģ��ʵ��
src/
-------->>gmsi_base.c/h
-------->>gmsi_input.c/h
-------->>gmsi_output.c/h
-------->>gmsi_uart.c/h
// �̳�gmsi/src�µ�ģ�����Ӳ��ʵ��
peripheral/
-------->>hclass.c/h
// gmsi���޹ؽӿ�ʵ�ֺ͵�������
utilities/
-------->>debug/
-------->>perf_counter/
-------->>PLOOC/
// gmsi���ͼ
image/
-------->>Base.drawio
-------->>framework.drawio
// ��дģ��(ע������Doxygen)
template/
-------->>gmsi_template.c/h
-------->>class_impl_template.c/h
-------->>class_app_template.c/h
```

+ **gmsi_lib**����Ӳ��ʵ�֣�����**Ӳ���м��**���ṩ**ͨ�ŷ���**
  + gmsi_uart:ʵ�ֳ������շ�����
  + gmsi_input:ʵ��io��ȡ����
    + �ڲ����˲���ز������õ�һ��Ӳ�������źţ������źŶ��ⷢ�ͼ�ֵ**event**
  + gmsi_output:��Ҫ�ṩӲ��ioд���������
    + �����ⲿ�����źţ������ź��������ƽ/pwm��

+ **peripheral**�淶��Ӳ����ؽӿڣ�����GMSI����������**Ӳ��ʵ����**
  + �̳�gmsi_lib������оƬ�⺯������**hardware_class**

+ **class**�Ǹ�����Ŀ��Ҫʵ�ֵĹ���ģ�黯
  + ��**peripheral**���ջ�����Ϣ


#### GMSI�����Ӧ��

##### SECTION�﷨

```c
typedef int (*gmsi_runfn_t)(void *ptObject);
typedef struct{
    void *ptObject;
    gmsi_runfn_t tfcnGmsiRun;
}gmsi_classrun_t;
// �����ⲿ����
extern gmsi_classrun_t CLASSRUN$$Base;
extern gmsi_classrun_t CLASSRUN$$Limit;
// ����gmsi_classrun_t����ָ�룬��ȡ�����ε���β��ַ
gmsi_classrun_t *ptGmsiAutoRunStart = &CLASSRUN$$Base;
gmsi_classrun_t *ptGmsiAutoRunEnd = &CLASSRUN$$Limit;
int gmsi_ClassRun(void)
{
    // ����Classrun�����ε����к���������
    for(gmsi_classrun_t *p=ptGmsiAutoRunStart;p<ptGmsiAutoRunEnd;p++)
    {
        p->tfcnGmsiRun(p->ptObject);
    }
    return 0;
}

// ֻҪ�������tfcnGmsiRun����int (*gmsi_runfn_t)(void *ptObject),�������
static gmsi_classrun_t GMSI_USED SECTION("CLASSRUN") tGmsiClassRunLedBreahe = {
    .ptObject = NULL,
    .tfcnGmsiRun = utildebug_LedBreathe
};

```

##### GMSI_ID

```c
/*
	1��ID������ṹ
	2����16bit����
	3����8bitΪ��
	4����8bitΪ����chNumber
*/ 
uint32_t wId;
//
// gmsiconfig.h
#define GMSI_ID_BASEOFFSET      8

#define GMSI_ID_INPUT           1
#define GMSI_ID_OUTPUT          2
#define GMSI_ID_UART            3
#define GMSI_ID_SPI             4
#define GMSI_ID_CAN             5
#define GMSI_ID_IIC             6
#define GMSI_ID_TIMER           7
#define GMSI_ID_CLASS           8

// ����base����ʱ��������������
gmsi_base_cfg_t tGbaseClassCfg = {
    .wId = GMSI_ID_CLASS << GMSI_ID_BASEOFFSET,
};
// ����������ýṹ��
typedef struct {
	// ��ı��Ψһֵ���߰�λΪ�ࣩ
    uint8_t chNumber;
}class_cfg_t;
// ������ʵ����ʱ�趨��ֵ
void class_Init(class_t *ptClass, class_cfg_t *ptCfg)
{
    // newһ��base
    ptClass->ptBase = gbase_New();
    // ȷ��ID��д��
    tGbaseClassCfg.wId +=  ptCfg->chNumber;
    // �����౸��Id
    ptClass->wId = tGbaseClassCfg.wId;
    // base init
    gbase_Init(ptClass->ptBase, &tGbaseClassCfg);
}
```

##### LISTͨ��ʵ��
###### LIST����
```c
/* 
	1������Ŀ��id�ű���������ȡ��Ӧ��base
	2���Ѷ�Ӧ��event��messageд���Ӧ��base
*/    
// gmsi_base.c
	// ��������ͷ��
struct xLIST        tListClass;

/*****************������������ start*****************/
	// ��������ȷ��Ŀ��id
    for(ptListDes = listGET_HEAD_ENTRY(&tListClass);        \
        ptListDes != NULL;                                  \
        ptListDes = listGET_NEXT(ptListDes)
        )
    {
        if(ptListDes->xItemValue == wId)
            break;
    }
    
    // �ҵ���Ӧ����ʵ��
    if(NULL != ptListDes)
    {
        ptBaseDes = ptListDes->pvOwner;
		// ��Ŀ��base����в���
    }
/*****************������������ end*****************/
```

###### GMSIͨ��

![Base](.assets/Base.jpg)

```c
typedef struct{
    /*��Ϣָ�� �������͸���Ӧ��ȷ�� ���֧���������*/
    void *vpMessage[5];
    /* ��Ϣ���� */
    uint16_t hwLength;
}message_t;
typedef struct {
    uint32_t wId;
    uint32_t wEvent;
    message_t *ptMessage;
    /* ���Ƶ�Id */
    // uint32_t wControlId;
    /* ���������������ڵ� */
    struct xLIST_ITEM   tListItem;
}gmsi_base_t;

// ������
gmsi_uart_data_t tGmsiUartData;
void class1_Run(class_t *ptClass)
{
    // eg:�Ӵ��ڵõ�һ֡����chArrayData,����Ϊ10
    tGmsiUartData.pchData = chArrayData;
    
    // ��ʽ1
    message_t tMessage = {
      	.vppmessage = &tGmsiUartData,
        .hwLength = 1
    };
    // ������д���ĳ��id
    gbase_MessagePost(ptClass->ptBase, ptClass->wConsumerId, &tMessage);
}
// ������
void class2_Run(class_t *ptClass)
{
	gmsi_uart_data_t *ptGmsiUartData = (gmsi_uart_data_t *)gbase_MessageGet(ptClass->ptBase);
    if(NULL != ptGmsiUartData)
    {
        // ���ݴ���
    }
}
```

##### GMSIͨ��

###### ͨ�ú���

+ �쳣�������

```c
// �����ӡ
void gcommon_PrintfError(gmsi_base_t *ptBase, greturn_error_t tGreturnErrorValue)
{
    // ������������Ǹ�����
    LOU_OUT(ptBase->wId);
    switch(tGreturnErrorValue)
    {
        case GMSI_ERROR_INIT:
            
            LOG_OUT("Init Error\r\n");
            break;
        default:
            LOG_OUT("Unknow Error\r\n");
            break;
    }
}
// �������ж���
void gcommon_PrintfObject(gmsi_base_t *ptBase)
{
    // UNUSE ptBase
    struct xLIST_ITEM *ptList;
    LOG_OUT("start list...\r\n");
    // ��������
    for(ptList = listGET_HEAD_ENTRY(&tListClass);        \
        ptList != NULL;                                  \
        ptList = listGET_NEXT(ptList)
        )
    {
        LOG_OUT(ptList->xItemValue);
        LOG_OUT("-->");
    }
    LOG_OUT("\r\n end list...\r\n");
}
```



###### ����ֵ

```c
// GMSI����ֵ
#define GMSI_ERROR_INIT		-1
#define GMSI_ERROR_MALLOC	-2

typedef int32_t greturn_error_t;
typedef uint32_t greturn_value_t;
typedef void* greturn_point_t;
```



#### ʹ�÷���

##### gmsi_xxxģ��

+ �����м�㹩**peripheral**ʵ��



##### classģ��

+ �����

```c
/*******************class.h start*************************/ 
// ����������ýṹ��
typedef struct {
	// ��ı��Ψһֵ���߰�λΪ�ࣩ
    uint8_t chNumber;
    // ������id; eg:��Ҫ��ĳ����ȴ����ݻ��¼�
    uint32_t wProducerId;
    // ������id: eg:��Ҫ����ĳ���࣬���䷢�����ݻ��¼�
    uint32_t wConsumerId;
}class_cfg_t;
// ���������Ϊ�Ͷ���
typedef struct {
    // �������ࣺ����ͨ�źͻ�ȡ�ⲿ�ź�����
    gmsi_base_t tBase;
    // ��������
}class_t;
/*******************class.h end***************************/ 

/* class.c */
void class_Init(class_t *ptClass, class_cfg_t *ptCfg)
{}
int class_Run(void *pvClass)
{
    class_t *ptClass = (class_t *)pvClass;
    // ״̬������
    // �ɸ����ⲿ�ź��Լ����ݽ��ж���
}
void class_Clock(void *pvClass)
{
    class_t *ptClass = (class_t *)pvClass;
    // �ڲ�ʱ��������
}

/*****************��������API_START***********************/
void class_Aaa(class_t *ptClass);
void class_Bbb(class_t *ptClass);
void class_Ccc(class_t *ptClass);
/*****************��������API_END*************************/

/*******************ʵ������******************************/ 
// ����ʵ��
class_t tClass;
class_cfg_t tClassCfg = {
    .chNumber = 1;
  	/* �������� */  
};

// app.c
// �������к�����ʱ�Ӻ�������Ӧ�ĺ�����SECTION
static gmsi_classrun_t GMSI_USED SECTION("CLASSRUN") tGmsiClassRun = {
    .ptObject = tClass,
    .tfcnGmsiRun = class_Run
};
static gmsi_clockrun_t GMSI_USED SECTION("CLOCKRUN") tGmsiClockRuntFlashledClock = {
    .ptObject = &tClass,
    .tfcnGmsiRun = class_Clock
};

// ����������ʼ������
class_Init(&tClass ,&tClassCfg);
```



### ����Ҫ��

+ ʹ���ĸ��ո����TAB

+ ����ָ��

  + ָ��ʹ��ǰҪ�ж��Ƿ�Ϊ�գ�NULL��

+ ���ڲ���ֵ

  + ������ʹ��stdbool.h�ṩ�Ĳ�����

  + ������ֱ��ʹ��0��1��ʾfalse��true��Ӧ��ֱ��ʹ��false��true��

  + �����Ӧ��Cϵͳδ�ṩstdbool.h�������ж��壬ͨ����������

    + ```c
      typedef enum {
          false = 0,
          true = !false
      } bool;
      ```

  + **���Բ��������߼����ʽ����true���бȽ�**����Ϊ�ڲ�������ֻ��false��ȷ����ֵ��0����true�������κη���ֵ��ʾ���ǲ�ȷ���ģ����Ҫ�����true���бȽϡ�

+ ������������

  + ����

    + ����ʹ�������Զ��ﹹ��

    + ��������ĸ��д

    + �����뵥��֮��**������**���»��߸���

    + ʹ�ø�����������������

      + ʹ�ø���������������������ǰ׺��ʾ�������ͣ�

      | ����     | ǰ׺ | ע��         |
      | -------- | ---- | ------------ |
      | uint8_t  | ch   | byte         |
      | int8_t   | c    |              |
      | uint16_t | hw   | half-word    |
      | int16_t  | i    |              |
      | uint32_t | w    | word         |
      | int32_t  | n    |              |
      | uint64_t | dw   | double-word  |
      | int64_t  | l    |              |
      | float    | f    |              |
      | double   | df   | double-float |
      | bool     | b    | boolean      |
      | ����ָ�� | fcn  |              |

      + ָ���ǰ׺�ǡ�p����ָ��ָ���ָ���ǡ�pp�����Դ����ƣ�һ���ò�����pp�����ϵġ�����Ǻ���ָ�룬����fcn

      + �����Զ���ı�������ǰ׺���ǡ�t��

      + �����������εı����ټ��벻ͬ��ǰ׺

        + static�ͱ���׷�ӡ�s_��

        + ȫ�ֱ���׷�ӡ�g_��

        + const���εı���׷�ӡ�c_������const��staticͬʱ����ʹ�á�c_��

          ```c
          bool g_bFlag = false;			//!< ȫ�ֱ���
          static uint16_t s_hwValue;		//!< ��̬����
          
          typedef struct {
              ��
          }example_t;
          
          {
              example_t tExample ;		//!< �ֲ�����
          ��
          }
          ```

      + �꣨Macro����ö�٣�enum��

        + �ɶ��ﹹ����һ��**��д**
        + �����뵥��֮�����»��߸���
        + �з���ֵ�ĺ꣬��ʹ�����Ű�������������/�������⣩
        + û�з���ֵ�ĺ꣬��ʹ�� do {} while(0) ��������

      + ������Function��

        + �ɶ��ﹹ����ԭ����һ��**Сд**
        + ģ��������������֮�����»��߸���������������������ĸ��д

  + ����

    + ����ͳ������С�==�����㣬����Ӧ�÷ŵ����ʽ�����

### ע��ģ��Doxygen

+ �ļ�ͷ��

```c
/**
* @file         
* @brief		This is a brief description.
* @details	    This is the detail description.
* @author		author
* @date		    date
* @version	    v1.0
* @par Copyright(c): 	abc corporation
* @par History:         
*	version: author, date, desc\n
*/
```

+ ����ע��

```c
 /**
 * @brief		This is a brief description.
 * @details	    This is the detail description. 
 * @param[in]	inArgName input argument description.
 * @param[out]	outArgName output argument description. 
 * @retval		0		�ɹ�
 * @retval		ERROR	���� 
 * @par ��ʶ��
 * 		����
 * @par ����
 * 		��
 * @par �޸���־
 * 		XXX��2023-10-03����
 */
greturn_error_t gmsi_xxxInit(gmsi_xxx_t *ptxxx, gmsi_xxx_cfg_t *ptCfg);
```

+ ���ݽṹ

```c
/** 
 * @brief		This is a brief description.
 * @details	    This is the detail description. 
 */
typedef struct
{
	int wVar1; /*!< Detailed description of the member var1 */
	int wVar2; /*!< Detailed description of the member var2*/
	int wVar3; /*!< Detailed description of the member var3 */
} gmsi_xxx_t;

```

+ ��ע��

```c
// �Ϸ������ҷ��Կ�
/** Description of the macro a */
#define a		0

#define b		0  /*!< Description of the macro b */

```

+ ȫ�ֺ;�̬����ע��

```c
/**  Description of global variable  */
int g_qwe = 0;
 
int static  s_asd = 0; /*!< Description of static variable */
```

+ ���ñ�ǩ����ؼ���
  + �ļ���Ϣ��
    + @file--> �ļ�����������ǰ�ļ���
    + @author --> ����
    + @todo --> �Ľ�������ָ����Եİ汾
  + ģ����Ϣ��
    + @var --> ģ�����˵��
    + @typedef --> ģ���������˵��
  + ������Ϣ��
    + @param --> ����˵��
    + @arg --> �б�˵��������Ϣ
    + @return --> ����ֵ˵��
    + @retval --> ����ֵ����˵��
    + @note --> ע��
  + ������Ϣ��
    + @brief --> ժҪ������ǰ�ļ�˵��
    + @see --> �ο�
    + @attention --> ע��
    + @bug --> ����
    + @warning --> ����
    + @sa --> �ο�����
