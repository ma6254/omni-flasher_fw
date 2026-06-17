# OmniFlasher firmware

## testing & build

```bash
pytest pytest_main.py -k test_hello_world --embedded-services esp,idf  --target esp32s3 --port COM4
```

```bash
idf.py build
```

## GPIO_MAP

| GPIO_NUM | Function   | Mode |
| -------- | ---------- | ---- |
| GPIO0    | BOOT       | IN   |
| GPIO3    | JTAG_EN    | IN   |
| GPIO19   | USB_D-     | IO   |
| GPIO20   | USB_D+     | IO   |
| GPIO21   | BUZZER     | OUT  |
| GPIO48   | WS2812_LED | OUT  |
|          | SWDIO      | IO   |
|          | SWCLK      | OUT  |
|          | TAR_RST    | OUT  |
|          | TAR_TDI    | IN   |
|          | TAR_TDO    | OUT  |
|          | TAR_IC1    | IO   |
|          | TAR_IC2    | IO   |
|          | TAR_IC3    | IO   |
|          | TAR_IC5    | IO   |
|          | TAR_IC6    | IO   |
|          | TAR_IC7    | IO   |
|          | TAR_IC9    | IO   |
|          | TAR_IC10   | IO   |
|          | TAR_IC11   | IO   |
|          | TAR_IC13   | IO   |
|          | TAR_IC14   | IO   |
|          | TAR_IC15   | IO   |
|          | U1TX       | OUT  |
|          | U1RX       | IN   |

## References

- **ESP32S3 Official Documentation** <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/>
