

#ifndef UTIL_SYSTEM_H
#define UTIL_SYSTEM_H


u32   sys_uptime();
u32   sys_free();
void *sys_getpid();
void  sys_reboot();

int fs_mount( const char *label, const char *target);
int fs_umount(const char *label);
int fs_format(const char *label);
int fs_partinfo();

str idf_version();
str rom_checksum(char *buf, uint len);

u32 flash_size();
u64 flash_id();

uint xtal_mhz();
uint cpu_mhz();
str  cpu_model();
uint cpu_revision();
uint cpu_cores();


#endif


