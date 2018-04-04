# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(IDF_PATH)/components/newlib/include $(IDF_PATH)/components/newlib/platform_include
COMPONENT_LDFLAGS += $(IDF_PATH)/components/newlib/lib/libc-psram-workaround.a $(IDF_PATH)/components/newlib/lib/libm-psram-workaround.a -lnewlib
COMPONENT_LINKER_DEPS += $(IDF_PATH)/components/newlib/lib/libc-psram-workaround.a $(IDF_PATH)/components/newlib/lib/libm-psram-workaround.a
COMPONENT_SUBMODULES += 
newlib-build: 
