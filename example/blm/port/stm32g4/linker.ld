/* BLM Bootloader Linker Script Template */
/* Target: Cortex-M (generic) */
/* Bootloader: 8KB at 0x08000000 */

/* Memory layout */
MEMORY
{
    FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 8K
    SHARED (rw)     : ORIGIN = 0x08002000, LENGTH = 1K
    RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 16K
}

/* Entry point */
ENTRY(Reset_Handler)

/* Stack size */
_Min_Stack_Size = 0x400;

SECTIONS
{
    /* Vector table - must be at start of flash */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    /* Code */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)

        KEEP(*(.init))
        KEEP(*(.fini))

        . = ALIGN(4);
        _etext = .;
    } >FLASH

    /* MODUS init section */
    init_infos :
    {
        . = ALIGN(4);
        __start_init_infos = .;
        KEEP(*(init_infos))
        __stop_init_infos = .;
        . = ALIGN(4);
    } >FLASH

    /* Read-only data */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    /* ARM exception handling */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >FLASH
    
    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    /* Shared info region (fixed address) */
    .shared_info (NOLOAD) :
    {
        . = ALIGN(4);
        *(.shared_info)
        . = ALIGN(4);
    } >SHARED

    /* Initialized data - copied to RAM at startup */
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >RAM AT> FLASH

    /* Uninitialized data */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } >RAM

    /* Stack */
    ._user_stack :
    {
        . = ALIGN(8);
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
        _estack = .;
    } >RAM

    /* Discard unused */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
