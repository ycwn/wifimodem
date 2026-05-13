

#ifndef CORE_LOGGER_H
#define CORE_LOGGER_H


enum {
	LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR,
	LOG_NUM,

	LOG_DIRECT = 1 << 8

};

void log_init();
void log_clear();
bool log_get(int opt);
void log_set(int opt, bool set);
str  log_str();
void log_printf( uint level, const char *msg, ...) __attribute__((format(printf, 2, 3)));
void log_puts(   uint level, const char *msg, uint len);
void log_hexdump(uint level, const void *buf, size_t len);


#ifdef DEBUG
#define LOGD(...)   do { log_printf( LOG_DEBUG, __VA_ARGS__); } while (0)
#define DUMP(b, l)  do { log_hexdump(LOG_DEBUG, (b), (l));    } while (0)
#else
#define LOGD(...)   do { } while (0)
#define DUMP(b, l)  do { } while (0)
#endif

#define LOGI(...)  do { log_printf(LOG_INFO,  __VA_ARGS__); } while (0)
#define LOGW(...)  do { log_printf(LOG_WARN,  __VA_ARGS__); } while (0)
#define LOGE(...)  do { log_printf(LOG_ERROR, __VA_ARGS__); } while (0)


#endif


