file(
  GLOB
  ASSETS_SRC_FILES
  ../../assets/startup_screen/c/startup_gif.c
  ../../assets/main_menu_screen/c/flash_icon.c
  ../../assets/main_menu_screen/c/serial_icon.c
  ../../assets/main_menu_screen/c/sd_card_icon.c
  ../../assets/main_menu_screen/c/settings_icon.c
  ../../assets/main_menu_screen/c/info_icon.c
  ../../assets/main_menu_screen/c/jtag_icon.c
)

file(
    GLOB
    SRC_FILES
    *.c
    ../components/*.c
    ../components/utils/*.c
    ../components/key/*.c
    ../components/buzzer/*.c
    ../screens/*.c
)

idf_component_register(
    SRCS
        ${SRC_FILES}
        ${ASSETS_SRC_FILES}
    INCLUDE_DIRS
        .
        ../../assets
        ../components
        ../components/utils
        ../components/key
        ../components/buzzer
        ../screens
    PRIV_REQUIRES
        esp_driver_gpio
        spi_flash
        nvs_flash
        esp_timer
        app_update
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_LVGL_H_INCLUDE_SIMPLE=1)