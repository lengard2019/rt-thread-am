MEMORY {
    FLASH (rx) : ORIGIN = 0x30000000, LENGTH = 0x1000000
    SDRAM (rwx) : ORIGIN = 0xa0000000, LENGTH = 0x4000000
}

SECTIONS {
  .data.extra : ALIGN(8) {

    __fsymtab_start = .;
    KEEP(*(FSymTab))
    __fsymtab_end = .;
    . = ALIGN(8);
    __vsymtab_start = .;
    KEEP(*(VSymTab))
    __vsymtab_end = .;
    . = ALIGN(8);

    . = ALIGN(8);
    __rt_init_start = .;
    KEEP(*(SORT(.rti_fn*)))
    __rt_init_end = .;
    . = ALIGN(8);

    __rt_utest_tc_tab_start = .;
    KEEP(*(UtestTcTab))
    __rt_utest_tc_tab_end = .;

    . = ALIGN(8);
    __am_apps_data_start = .;
    *(__am_apps.data*)
    *(__am_apps.sdata*)
    __am_apps_data_end = .;
    . = ALIGN(8);
  } > SDRAM AT > FLASH
  
  _data_extra_start = ADDR(.data.extra);
  _data_extra_end = _data_extra_start + SIZEOF(.data.extra);
  _data_extra_load = LOADADDR(.data.extra);
} 
INSERT BEFORE .data;

SECTIONS {
  .bss.extra (NOLOAD) : ALIGN(8){
    . = ALIGN(8);
    __am_apps_bss_start = .;
    *(__am_apps.bss*)
    *(__am_apps.sbss*)
    *(__am_apps.scommon*)
    __am_apps_bss_end = .;
    . = ALIGN(8);
  } > SDRAM
  _bss_extra_start = ADDR(.bss.extra);
  _bss_extra_end = _bss_extra_start + SIZEOF(.bss.extra);
  /* _bss_extra_load = LOADADDR(.bss.extra); */
} 
INSERT BEFORE .bss;
