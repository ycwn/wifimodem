

#ifndef COROUTINE_H
#define COROUTINE_H


#ifdef COROUTINE_COMPAT

#define CO_RESET(S)    do { (S) = 0; } while (0)
#define CO_BEGIN(S)    CO_STATE *COROUTINE_STATE = &(S); switch (*COROUTINE_STATE) { case 0:
#define CO_END()       *COROUTINE_STATE = __LINE__; case __LINE__: break; }
#define CO_YIELD(...)  do { *COROUTINE_STATE = __LINE__; return __VA_ARGS__; case __LINE__:; } while (0)
#define CO_SYNC()      do { *COROUTINE_STATE = __LINE__; case __LINE__:; } while (0)
#define CO_ISRESET(S)  ((S) == 0)

typedef int CO_STATE;

#else

#define CO_RESET(S)    do { (S) = NULL; } while (0)
#define CO_BEGIN(S)    CO_STATE *COROUTINE_STATE = &(S); if (*COROUTINE_STATE != NULL) goto **COROUTINE_STATE;
#define CO_END()
#define CO_YIELD(...)  do { __label__ next_; *COROUTINE_STATE = &&next_; return __VA_ARGS__; next_:; } while (0)
#define CO_SYNC()      do { __label__ next_; *COROUTINE_STATE = &&next_; next_:; } while (0)
#define CO_ISRESET(S)  ((S) == NULL)

typedef void *CO_STATE;

#endif


#endif


