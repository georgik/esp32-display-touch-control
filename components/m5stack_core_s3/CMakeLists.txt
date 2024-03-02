#IDF version is less than IDF5.0
if("${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}" VERSION_LESS "5.0")
    set(SRC_VER "m5stack_core_s3_idf4.c")
else()
    set(SRC_VER "m5stack_core_s3_idf5.c")
endif()

idf_component_register(
    SRCS "m5stack_core_s3.c" ${SRC_VER}
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver spiffs
    PRIV_REQUIRES fatfs esp_lcd
)
