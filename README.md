# OmniFlasher firmware

## testing & build

### Comparison of ESP32-S3 series chips

Note: Different models of ESP32S3 chips correspond to different types of PSRAM

- _ESP32S3_: No PSRAM
- _ESP32S3-R2_: 2 MB (Quad SPI)
- _ESP32S3-R8_: 8 MB (Octal SPI)

### Build & Flash

```bash
# convert images to C source code

# startup_screen
python managed_components/lvgl__lvgl/scripts/LVGLImage.py --ofmt C --cf RAW -o ./assets/startup_screen/c ./assets/startup_screen/startup_gif.gif  


# main_menu_screen
python managed_components/lvgl__lvgl/scripts/LVGLImage.py --ofmt C --cf RGB565A8 -o ./assets/main_menu_screen/c ./assets/main_menu_screen/flash_icon.gif
python managed_components/lvgl__lvgl/scripts/LVGLImage.py --ofmt C --cf RGB565A8 -o ./assets/main_menu_screen/c ./assets/main_menu_screen/serial_icon.gif
python managed_components/lvgl__lvgl/scripts/LVGLImage.py --ofmt C --cf RGB565A8 -o ./assets/main_menu_screen/c ./assets/main_menu_screen/sd_card_icon.gif
python managed_components/lvgl__lvgl/scripts/LVGLImage.py --ofmt C --cf RGB565A8 -o ./assets/main_menu_screen/c ./assets/main_menu_screen/settings_icon.gif
python managed_components/lvgl__lvgl/scripts/LVGLImage.py --ofmt C --cf RGB565A8 -o ./assets/main_menu_screen/c ./assets/main_menu_screen/info_icon.gif
python managed_components/lvgl__lvgl/scripts/LVGLImage.py --ofmt C --cf RGB565A8 -o ./assets/main_menu_screen/c ./assets/main_menu_screen/jtag_icon.gif

```

```bash
pytest pytest_main.py -k test_hello_world --embedded-services esp,idf  --target esp32s3 --port COM4
```

```bash
idf.py build
idf.py flash
```

### LVGL-Benchmark

```bash
lvgl_benchmark: ========== Benchmark Results ==========
lvgl_benchmark: Scene                               FPS   CPU% Render(ms)  Flush(ms)
lvgl_benchmark: -----------------------------------------------------------------------
lvgl_benchmark: Empty screen                         33     69         19          0
lvgl_benchmark: Moving wallpaper                     33     66         18          2
lvgl_benchmark: Single rectangle                     96      9          0          0
lvgl_benchmark: Multiple rectangles                  48     46          0         10
lvgl_benchmark: Multiple RGB images                  87      7          0          0
lvgl_benchmark: Multiple ARGB images                 57     34          0          3
lvgl_benchmark: Rotated ARGB images                  33     61         11          8
lvgl_benchmark: Multiple labels                      50     51          0          0
lvgl_benchmark: Screen sized text                    16     81         51          0
lvgl_benchmark: Multiple arcs                        96     12          0          0
lvgl_benchmark: Containers                           57     20         10          0
lvgl_benchmark: Containers with overlay              32     61         18          2
lvgl_benchmark: Containers with opa                  57     25         10          0
lvgl_benchmark: Containers with opa_layer            47     40         18          0
lvgl_benchmark: Containers with scrolling            31     64         15          5
lvgl_benchmark: Widgets demo                         21     75         16          1
lvgl_benchmark: -----------------------------------------------------------------------
lvgl_benchmark: Valid scenes : 16
lvgl_benchmark: Avg FPS      : 49
lvgl_benchmark: Avg CPU%     : 45
lvgl_benchmark: Avg Render   : 11 ms
lvgl_benchmark: Avg Flush    : 1 ms
lvgl_benchmark: =======================================
```

## GPIO_MAP

| GPIO_NUM | Label       | Mode | Function      |
| -------- | ----------- | ---- | ------------- |
| GPIO0    | BOOT        | IN   |               |
| GPIO1    | IC_VCC_EN   | OUT  |               |
| GPIO2    | JTAG_VCC_EN | OUT  |               |
| GPIO3    | JTAG_EN     | IN   |               |
| GPIO4    | IC_OE1      | OUT  |               |
| GPIO5    | PMIC_SCL    | OUT  |               |
| GPIO6    | IC_OE2      | OUT  |               |
| GPIO7    | PMIC_SDA    | OUT  |               |
| GPIO8    | IC_OE3      | OUT  |               |
| GPIO9    | LCD_BL      | OUT  | LEDC_PWM_CH1  |
| GPIO10   | PMIC_IRQ    | IN   | INTR          |
| GPIO11   | LCD_MOSI    | OUT  | SPI2_MOSI     |
| GPIO12   | LCD_DC      | OUT  | GPIO          |
| GPIO13   | LCD_SCLK    | OUT  | SPI2_CLK      |
| GPIO14   | LCD_CS      | OUT  | SPI2_CS       |
| GPIO15   | LCD_RST     | OUT  |               |
| GPIO16   | KEY3        | IN   |               |
| GPIO17   | KEY2        | IN   |               |
| GPIO18   | KEY1        | IN   |               |
| GPIO19   | USB_D-      | IO   | USB           |
| GPIO20   | USB_D+      | IO   | USB           |
| GPIO21   | BUZZER      | OUT  | PWM           |
| GPIO33   | IC_A6       | IO   |               |
| GPIO34   | IC_A5       | IO   |               |
| GPIO35   | IC_A4       | IO   |               |
| GPIO36   | IC_A3       | IO   |               |
| GPIO37   | IC_A2       | IO   |               |
| GPIO38   | IC_A1       | IO   |               |
| GPIO47   | IC_A7       | IO   |               |
| GPIO48   | WS2812_LED  | OUT  | LED_STRIP_RMT |

### Display board interface define

| GPIO   | Label   | NUM | NUM | Label   | GPIO   |
| ------ | ------- | --- | --- | ------- | ------ |
| GPIO15 | LCD_RST | 12  | 1   | LCD_SDA | GPIO11 |
| GPIO48 | LED     | 10  | 9   | LCD_SCL | GPIO13 |
| GPIO18 | K1      | 8   | 7   | LCD_CS  | GPIO14 |
| GPIO17 | K2      | 6   | 5   | LCD_RS  | GPIO12 |
| GPIO16 | K3      | 4   | 3   | LCD_BL  | GPIO9  |
|        | 3V3     | 2   | 1   | GND     |        |

### SPI Flash Mode

| Chan  | Label | Mode | Function |
| ----- | ----- | ---- | -------- |
| IC_A1 | CS    | OUT  | SPI_CS   |
| IC_A2 | IO3   | IO   | SPI_IO3  |
| IC_A3 | IO1   | IO   | SPI_IO1  |
| IC_A4 | CLK   | OUT  | SPI_CLK  |
| IC_A5 | IO2   | IO   | SPI_IO2  |
| IC_A6 | IO0   | IO   | SPI_IO0  |

### I2C EEPROM Mode

| Chan  | Label | Mode | Function |
| ----- | ----- | ---- | -------- |
| IC_A1 | A0    | OUT  |          |
| IC_A2 | WP    | OUT  |          |
| IC_A3 | A1    | OUT  |          |
| IC_A4 | SCL   | OUT  | I2C_SCL  |
| IC_A5 | A2    | OUT  |          |
| IC_A6 | SDA   | IO   | I2C_SDA  |

### JTAG & SWD Mode

| Chan  | Label | Mode | Function |
| ----- | ----- | ---- | -------- |
| IC_A1 | SWCLK | OUT  | SPI_CLK  |
| IC_A2 | SWDIO | IO   | SPI_IO   |
| IC_A3 | TDI   | IN   |          |
| IC_A4 | TDO   | OUT  |          |
| IC_A5 | RST   | OUT  |          |
| IC_A6 | UTXD  | IO   | UART_TXD |
| IC_A7 | URXD  | IO   | UART_RXD |

## References

- **ESP32S3 Official Documentation** <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/>
- **LVGL Image Converter** <https://lvgl.io/tools/imageconverter>
- **Google Fonts Icons** <https://fonts.google.com/icons>
