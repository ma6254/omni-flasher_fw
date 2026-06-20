# OmniFlasher firmware

## testing & build

```bash
pytest pytest_main.py -k test_hello_world --embedded-services esp,idf  --target esp32s3 --port COM4
```

```bash
idf.py build
```

lv_demo_benchmark

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

| GPIO_NUM | Label      | Mode | Function      |
| -------- | ---------- | ---- | ------------- |
| GPIO0    | BOOT       | IN   |               |
| GPIO3    | JTAG_EN    | IN   |               |
| GPIO19   | USB_D-     | IO   | USB           |
| GPIO20   | USB_D+     | IO   | USB           |
| GPIO21   | BUZZER     | OUT  | PWM           |
| GPIO48   | WS2812_LED | OUT  | LED_STRIP_RMT |
|          | LCD_BL     | OUT  | PWM           |
|          | LCD_SCL    | OUT  | SPI_CLK       |
|          | LCD_SDA    | OUT  | SPI_MOSI      |
|          | LCD_CS     | OUT  | SPI_CS        |
|          | LCD_RST    | OUT  | GPIO          |
|          | LCD_RS     | OUT  | GPIO          |
|          | SWDIO      | IO   |               |
|          | SWCLK      | OUT  |               |
|          | TAR_RST    | OUT  | GPIO          |
|          | TAR_TDI    | IN   |               |
|          | TAR_TDO    | OUT  |               |
|          | TAR_IC1    | IO   |               |
|          | TAR_IC2    | IO   |               |
|          | TAR_IC3    | IO   |               |
|          | TAR_IC5    | IO   |               |
|          | TAR_IC6    | IO   |               |
|          | TAR_IC7    | IO   |               |
|          | TAR_IC9    | IO   |               |
|          | TAR_IC10   | IO   |               |
|          | TAR_IC11   | IO   |               |
|          | TAR_IC13   | IO   |               |
|          | TAR_IC14   | IO   |               |
|          | TAR_IC15   | IO   |               |
|          | U1TX       | OUT  |               |
|          | U1RX       | IN   |               |

## References

- **ESP32S3 Official Documentation** <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/>
