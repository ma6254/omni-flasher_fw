# OmniFlasher firmware

## testing & build

```bash
pytest pytest_main.py -k test_hello_world --embedded-services esp,idf  --target esp32s3 --port COM4 
```

```bash
idf.py build
```

## References

- **ESP32S3 Official Documentation** <https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/>
