file(
    GLOB
    SRC_FILES
    *.c
)

idf_component_register(
    SRCS
        ${SRC_FILES}
    INCLUDE_DIRS
        .
    PRIV_REQUIRES
        esp_driver_gpio
        spi_flash
        nvs_flash
        esp_timer
        app_update
)
