deps_config := \
	/opt/esp-idf/components/aws_iot/Kconfig \
	/opt/esp-idf/components/bt/Kconfig \
	/opt/esp-idf/components/esp32/Kconfig \
	/opt/esp-idf/components/ethernet/Kconfig \
	/opt/esp-idf/components/fatfs/Kconfig \
	/opt/esp-idf/components/freertos/Kconfig \
	/opt/esp-idf/components/log/Kconfig \
	/opt/esp-idf/components/lwip/Kconfig \
	/opt/esp-idf/components/mbedtls/Kconfig \
	/opt/esp-idf/components/openssl/Kconfig \
	/opt/esp-idf/components/spi_flash/Kconfig \
	/opt/esp-idf/components/bootloader/Kconfig.projbuild \
	/opt/esp-idf/components/esptool_py/Kconfig.projbuild \
	/opt/esp-idf/components/partition_table/Kconfig.projbuild \
	/opt/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
