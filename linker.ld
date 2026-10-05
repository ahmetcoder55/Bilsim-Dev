ENTRY(_start)

SECTIONS
{
    . = 0x1000;
    
    .text : AT(0x1000)
    {
        _code_start = .;
        KEEP(*(.text.start))
        *(.text)
        *(.rodata*) 
        . = ALIGN(4096);
        _code_end = .;
    }
    
    .data : ALIGN(4096)
    {
        _data_start = .;
        *(.data)
        . = ALIGN(4096);
        _data_end = .;
    }
    
    .bss : ALIGN(4096)
    {
        _bss_start = .;
        *(.common)
        *(.bss)
        . = ALIGN(4096);
        _bss_end = .;
    }

    _kernel_final_mark = .;
}
