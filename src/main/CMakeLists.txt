file(
  GLOB
  ASSETS_SRC_FILES

  # startup_screen
  ../../assets/startup_screen/c/startup_gif.c
  ../../assets/main_menu_screen/c/remote_icon.c
  ../../assets/main_menu_screen/c/flash_icon.c
  ../../assets/main_menu_screen/c/serial_icon.c
  ../../assets/main_menu_screen/c/sd_card_icon.c
  ../../assets/main_menu_screen/c/settings_icon.c
  ../../assets/main_menu_screen/c/info_icon.c
  ../../assets/main_menu_screen/c/jtag_icon.c
  
  # remote_screen
  ../../assets/remote_screen/c/usb_icon.c
  ../../assets/remote_screen/c/wifi_icon.c
  ../../assets/remote_screen/c/flashrom_logo.c

  # settings_screen
  ../../assets/settings_screen/c/sun_icon.c
  
  # fonts
  ../../assets/fonts/c/main_menu_screen_item_font.c
  ../../assets/fonts/c/remote_screen_item_font.c
  ../../assets/fonts/c/serial_screen_item_font.c
  ../../assets/fonts/c/settings_screen_item_font.c
)

file(
  GLOB
  TS_SRC_FILES
  ../../translations/c/*.c
)

file(
    GLOB
    SRC_FILES
    *.c
    ../components/*.c
    ../components/utils/*.c
    ../components/key/*.c
    ../components/buzzer/*.c
    ../components/config/*.c
    ../screens/components/lv_exmenu/*.c
    ../screens/*.c
)

idf_component_register(
    SRCS
        ${SRC_FILES}
        ${ASSETS_SRC_FILES}
        ${TS_SRC_FILES}
    INCLUDE_DIRS
        .
        ../../assets
        ../../translations/c
        ../components
        ../components/utils
        ../components/key
        ../components/buzzer
        ../components/config
        ../screens/components
        ../screens/components/lv_exmenu
        ../screens
    PRIV_REQUIRES
        esp_driver_gpio
        spi_flash
        nvs_flash
        esp_timer
        app_update
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_LVGL_H_INCLUDE_SIMPLE=1)