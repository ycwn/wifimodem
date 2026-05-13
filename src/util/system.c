
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <esp_system.h>
#include <esp_timer.h>
#include <esp_mac.h>
#include <esp_flash.h>
#include <esp_spiffs.h>
#include <esp_chip_info.h>
#include <esp_ota_ops.h>
#include <esp_private/esp_clk.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"

#include "util/system.h"


static const str model[]={
	CSTR("ESP32"),     CSTR("ESP32-S2"), CSTR("ESP32-S3"), CSTR("ESP32-C3"),
	CSTR("ESP32-C2"),  CSTR("ESP32-C6"), CSTR("ESP32-H2"), CSTR("ESP32-P4"),
	CSTR("ESP32-C61"), CSTR("SIM"),      CSTR("UNKNOWN"),  CSTR("UNKNOWN"),
	CSTR("UNKNOWN"),   CSTR("UNKNOWN"),  CSTR("UNKNOWN"),  CSTR("UNKNOWN")
};



u32 sys_uptime()
{
	return (u64)esp_timer_get_time() / 1000UL;
}



u32 sys_free()
{
	return esp_get_free_heap_size();
}



void *sys_getpid()
{
	return xTaskGetCurrentTaskHandle();
}



void sys_reboot()
{

	esp_restart();

}



int fs_mount(const char *label, const char *target)
{

	esp_vfs_spiffs_conf_t config = {
		.base_path              = target,
		.partition_label        = label,
		.max_files              = 5,
		.format_if_mount_failed = false,
	};

	return -esp_vfs_spiffs_register(&config);

}



int fs_umount(const char *label)
{

	return -esp_vfs_spiffs_unregister(label);

}



int fs_format(const char *label)
{

	return -esp_spiffs_format(label);

}



str idf_version()
{

	const char *espver = esp_get_idf_version();
	return CSTRZ(espver);

}



str rom_checksum(char *buf, uint len)
{

	esp_app_get_elf_sha256(buf, len);
	return CSTRZ(buf);

}



u32 flash_size()
{
	u32 size = 0;
	esp_flash_get_physical_size(NULL, &size);
	return size;
}



u64 flash_id()
{
	u64 id = 0;
	esp_flash_read_unique_chip_id(NULL, &id);
	return id;
}



uint xtal_mhz()
{
	return esp_clk_xtal_freq();
}



uint cpu_mhz()
{
	return esp_clk_cpu_freq();
}



str cpu_model()
{
	esp_chip_info_t info;
	esp_chip_info(&info);
	return model[info.model & 15];
}



uint cpu_revision()
{
	esp_chip_info_t info;
	esp_chip_info(&info);
	return info.revision;
}



uint cpu_cores()
{
	esp_chip_info_t info;
	esp_chip_info(&info);
	return info.cores;
}


