add_firmware(
    NAME seven_seg_digit
    BOARD ${BOARD}
    SOURCES src/main.c
    DRIVERS driver_gpio driver_rcc
)

add_flash_target(seven_seg_digit ${BOARD})
add_debug_target(seven_seg_digit ${BOARD})
