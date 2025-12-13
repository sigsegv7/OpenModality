ENTRY(_start)

SECTIONS {
    . = 0x1000;

    .text : ALIGN(8) {
        *(.text .text.*)
    }

    .data : ALIGN(8) {
        *(.data .data.*)
    }

    .rodata : ALIGN(8) {
        *(.rodata .rodata.*)
    }

    .bss : ALIGN(8) {
        *(.bss .bss.*)
    }

    /DISCARD/ : {
        *(.eh_frame .eh_frame.*)
        *(.note .note.*)
    }
}
