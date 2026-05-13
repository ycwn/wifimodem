

#ifndef CORE_DEBUG_H
#define CORE_DEBUG_H


#undef checkpoint
#undef assert
#undef watch


#ifdef DEBUG

#define checkpoint() \
	do { \
		LOGD("%s:%d: check!\n", __FILE__, __LINE__); \
	} while (0)

#define fixme(msg) \
	do { \
		LOGD("%s:%d: FIXME:" msg "\n", __FILE__, __LINE__); \
	} while (0)

#define todo(msg) \
	do { \
		LOGD("%s:%d: TODO:" msg "\n", __FILE__, __LINE__); \
	} while (0)

#define assert(pred) \
	do { \
		if (!(pred)) \
			LOGD("%s:%d: assertion '%s' failed!\n", __FILE__, __LINE__, #pred); \
	} while (0)

#define watch(format, var...) \
	do { \
		LOGD("%s:%d: %s = " format "\n", __FILE__, __LINE__, #var, var); \
	} while(0)

#else

#define checkpoint()        do { /* Nothing */ } while (0)
#define fixme(msg)          do { /* Nothing */ } while (0)
#define todo(msg)           do { /* Nothing */ } while (0)
#define assert(pred)        do { /* Nothing */ } while (0)
#define watch(format, ...)  do { /* Nothing */ } while (0)

#endif


#endif


