set(srcs "main.c"
    "st7789.c"
    "fontx.c"
    "esp_hid_gap.c"
    "keymapping.c"
    )



idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES esp_hid)
