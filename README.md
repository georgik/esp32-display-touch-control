# ESP32 Display with touch control

Experimental implementation.

## Build


M5Stack CoreS3
```
idf.py set-target esp32s3
idf.py -D  SDKCONFIG_DEFAULTS=sdkconfig.bsp.m5stack_core_s3 build flash monitor
```

