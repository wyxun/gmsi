## GMSI���

**Generic Mcu Software Infrastructure**

![framework](.assets/framework.jpg)

### �ļ��ֲ����

```c
// gmsi��ʵ���ļ�
gmsi
--->>gmsi.c/h
--->>base.c/h
--->>global_define.h
// gmsi���
--->>component/
----------->>gmsi_uart.c/h
--->>utilities/
----------->>utilities.h
----------->>list.c/h
----------->>util_queue.c/h
// ��������
ThirdParty/
--->>PLOOC/
// gmsi���ͼ
image/
-------->>Base.drawio
-------->>framework.drawio
// ��ͬƽ̨������(ע������Doxygen)
example/
// cortex_mƽ̨������
--->>cortex_m/
// linuxƽ̨������
--->>posix_uart/
// gmsiͨ��ģ��
--->>template/
```

### GMSI�����Ӧ��

#### object�໥ͨ�Ż���

```c
/***************************base.hʵ��******************************/
// object���ID
typedef struct{
    uint32_t wId;
    /* ... */
}gmsi_base_cfg_t;
/***************************global_define.hʵ��**********************/
// object ID ���໮��
#define GMSI_ID_UART        1
#define GMSI_ID_IIC         2
#define GMSI_ID_SPI         3
#define GMSI_ID_CAN         4
#define GMSI_ID_INPUT       5
#define GMSI_ID_SENSOR      6
#define GMSI_ID_TIMER       7
// SOFTWARE
#define GMSI_ID_CLASS       100
#define GMSI_ID_MOCK        101

/***************************userconfig.hʵ��**************************/
// ����һ������object ID
#define OBJECT     (GMSI_ID_MOCK<<8+1)
#define OBJECT2    (GMSI_ID_MOCK<<8+2)

/***************************object.cʵ��******************************/
// object init id
gmsi_base_cfg_t tObjectBaseCfg = {
    .wId = OBJECT,
    /* ... */
};
// gmsi base object init
int object_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr)
{
    wRet = gbase_Init(ptThis->ptBase, &tObjectBaseCfg);
    {
        ptBase->wId = ptCfg->wId;
    }
}
// object�¼�����ʵ��
int object_Run(uint32_t wObjectAddr)
{
    // wObjectAddrת����objectָ��
    // �ȴ������¼�����
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent)
        example_EventHandle(ptThis, wEvent);
}
int object_Clock(uint32_t wObjectAddr)
{
    // ��ʱ������object�����¼�
    gbase_EventPost(OBJECT2, Gmsi_Event00);
    {
        // ���������ҵ�OBJECT2 id
        if(ptListItemDes->xItemValue == wId)
        	ptBaseDes = ptListItemDes->pvOwner;
        // ��OBJECT2���¼���ֵ
        ptBaseDes->wEvent |= wEvent;
    }
}
```

#### object�Զ�����ʵ�ֻ���

```c
/***************************object.cʵ��******************************/
// gmsi base object init
gmsi_base_cfg_t s_tExampleBaseCfg = {
    .wId = EXAMPLE,
    /* ��ȡ��ָ�� */
    .wParent = 0,
    /* ��gmsi�Ľӿں�����ַд�뵽base_cfg_t */
    .FcnInterface = {
        .Clock = object_Clock,
        .Run = object_Run,
    },
};

// object init
int object_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr)
{
    // ��ʼ��Parent addr
    s_tObjectBaseCfg.wParent = wObjectAddr;
    wRet = gbase_Init(ptThis->ptBase, &s_tExampleBaseCfg);
}

/***************************gmsi.cʵ��******************************/
void gmsi_Run(void)
{
    // ��������ִ�ж�Ӧobject��Run����
	ptBaseDes->pFcnInterface->Run(ptBaseDes->wParent);
}
void gmsi_Clock(void)
{
	// ��������ִ�ж�Ӧobject��Clock����
	ptBaseDes->pFcnInterface->Clock(ptBaseDes->wParent);
}

```

#### LISTͨ��ʵ��
##### object���һ���
```c
/* 
	1������Ŀ��id�ű���������ȡ��Ӧ��base
	2���Ѷ�Ӧ��event��messageд���Ӧ��base
*/    
/***************************base.cʵ��******************************/
	// ��������ͷ��
struct xLIST        tListObject;

/*****************������������ start*****************/
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;
    uint8_t chErgodicTime = 1;
    gmsi_base_t *ptBaseDes;
    // ��������ȷ��Ŀ��id
    while(ptListItemDes != &tListObject.xListEnd){
        if(ptListItemDes->xItemValue == wId)
            break;
        // ��ƥ���������
        chErgodicTime++;
        ptListItemDes = ptListItemDes->pxPrevious;
    }
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {
        // �ҵ���Ӧ��object
        ptBaseDes = ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptBaseDes);
        // ������Ӧ��object
        ptBaseDes->tMessage.pchMessage= pchMessage;
        ptBaseDes->tMessage.hwLength = hwLength;
        ptBaseDes->wEvent |= Gmsi_Event_Transition;
    }
/*****************������������ end*****************/
```

#### GMSIͨ��

##### ͨ�ú���

+ �쳣�������

```c
// �����ӡ
void gmsi_errorlog(int wErrorNum)
{
    switch(wErrorNum)
    {
        case GMSI_SUCCESS:
        break;
        
        default:
        break;
    }
}
// �������ж���
void gbase_DegugListBase(void)
{
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;
    printf("List all object:\n");
    // ��������
    while(ptListItemDes != &tListObject.xListEnd){
        printf("    itme id: %d\n", ptListItemDes->xItemValue);
        ptListItemDes = ptListItemDes->pxPrevious;
    }
}
```



##### ����ֵ

```c
// GMSI����ֵ������LINUX
#define GMSI_SUCCESS        0
#define GMSI_EPERM          -1
#define GMSI_ENOENT         -2
#define GMSI_ESRCH          -3  
#define GMSI_EINTR          -4
```



## ʹ�÷���

**�ο�example/templateʵ��**

### ��Ŀ��������

#### llvm������+cubemx����

+ ����cubemx����gcc��Ŀ

+ �޸�makefile�ı�����

  ```makefile
  CC = $(llvm_path)\bin\clang.exe
  AS = $(llvm_path)\bin\clang.exe
  OBJCOPY = $(llvm_path)\bin\llvm-objcopy
  SIZE = $(llvm_path)\bin\llvm-size
  ```

+ �޸�CFLAG

  ```makefile
  CFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
  CFLAGS += -std=gnu11 -Wall -Wextra -Ofast
  CFLAGS += -fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing
  CFLAGS += -fshort-enums -fomit-frame-pointer -c -fdata-sections -ffunction-sections
  ```

+ ָ��ld�ļ�

  ```makefile
  LDSCRIPT = STM32G431.lds
  ```

#### llvm������+vscode�༭

+ �ڸ�Ŀ¼�´���project�ļ���

  ```sh
  mkdir project
  cd project
  ```

+ ����example/template��project�ļ��£�����template������Ϊ������

  ```sh
  cp -r ../example/template .
  mv template myproject
  ```

+ �޸�myproject��makefile�ļ�

  ```makefile
  #TARGET := demo
  TARGET := myproject				# �����Զ���
  
  # ָ����ĿԴ�ļ���оƬ�⡢�������ļ���
  # SOURCES += main.c example.c
  SOURCES += main.c object.c		# ���ⲹ�ϸ���Ŀ�Ķ����ļ�OBJECT.c����ָ���ж����ļ�
  
  # ָ����������������Ŀƽ̨���� Ƕ��ʽ/x86
  CC = $(llvm_path)\bin\clang.exe
  ```

#### keil+cubemx

+ ����cubemx����keil��Ŀ
+ ��keil��Ŀ������gmsi���ļ�

## MAKEFILE˵��

+ ָ��������

```makefile
// ʹ��x86�µ�llvm
CC := clang
CXX := clang++
// ʹ��Ƕ��ʽƽ̨��llvm
CC = $(llvm_path)\bin\clang.exe
AS = $(llvm_path)\bin\clang.exe
OBJCOPY = $(llvm_path)\bin\llvm-objcopy
SIZE = $(llvm_path)\bin\llvm-size
```

+ ����flag

```makefile
# CFLAG:c�ļ���־
CFLAGS := -std=c11 -W -Ofast
# ASFLAG������ļ���־������Ƕ��ʽƽ̨��Ҫ
ASFLAGS = -target armv7em-none-eabi -mthumb
# DFLAG�����־
DFLAG ��= -D_XOPEN_SOURCE=700 -DLINUX_POSIX
# LDFLAG:���ӱ�־
LDFLAGS += -T $(LDSCRIPT)			# Ƕ��ʽ��Ҫld�ļ���ָ������˳��
LDFLAGS +=-fno-autolink -Wall -lrt 	# ���Ӿ�̬��rt
```

+ Դ�ļ�·��

```makefile
# ��ѡ����Ŀ¼�����е�c�ļ���ӵ�SOURCES����
SOURCES += $(foreach dir, $(GMSI_DIR), $(wildcard $(dir)/*.c))
SOURCES += $(foreach dir, $(GMSI_UTL_DIR), $(wildcard $(dir)/*.c))
# ָ�����c�ļ�
SOURCES += main.c pc_uart.c pc_clock.c
```

+ ��������

```makefile
# ���ļ����ӳɿ�ִ���ļ�
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
# ��Դ�ļ�����ɻ��.o�ļ�
%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
```

+ ���

```makefile
# ɾ�����������ļ�
clean:
	rm -rf obj/$(COBJS) $(TARGET) $(OBJECTS)
# ��ӡmakefile�ļ���ı���ֵ
printf_value:
	@echo $(info source files is '$(SOURCES)')
	@echo $(info CFILENDIR files is '$(CFILENDIR)')
```



## ����Ҫ��

### �����ʽ

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

### ע��ģ��

**֧��Doxygen**

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
