add_firmware(
    NAME shift_register
    BOARD ${BOARD}
    SOURCES src/main.c
    DRIVERS driver_gpio driver_rcc
)

add_flash_target(shift_register ${BOARD})
add_debug_target(shift_register ${BOARD})
