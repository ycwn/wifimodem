

#ifndef UTIL_JSON_H
#define UTIL_JSON_H


enum {

	JSON_ERROR = -1,
	JSON_DONE  = 0,
	JSON_OK,

	JSON_BOOL,
	JSON_NUMBER,
	JSON_STRING,
	JSON_NULL,

	JSON_ARRAY_BEGIN,
	JSON_ARRAY_END,

	JSON_OBJECT_BEGIN,
	JSON_OBJECT_END,
	JSON_OBJECT_KEY,
	JSON_OBJECT_VALUE,

	JSON_TOKEN_MAX

};

enum {
	UJSON_ERROR = -1,
	UJSON_OK    =  0,
	UJSON_EMIT
};


typedef struct {

	buffer buffer;

	CO_STATE state;
	CO_STATE stack[8];

	int top;
	int final;

} json;



void        json_init( json *jsn);
int         json_parse(json *jsn, char ch);
const char *json_token(int id);

static inline void        json_setbuf(json *jsn, u8 *buf, uint len) { buffer_init(&jsn->buffer, buf, len); }
static inline const char *json_str(const json *jsn)                 { return buffer_rstr(&jsn->buffer);  }
static inline uint        json_len(const json *jsn)                 { return buffer_wtell(&jsn->buffer); }


#define UJSON_ADVANCE()  do { CO_YIELD(UJSON_OK); } while (0)
#define UJSON_EXPECT(t)  do { UJSON_CHECK(t); UJSON_ADVANCE(); } while (0)
#define UJSON_MATCH(s)   if (!strcmp(UJSON_STATE.str(), (s)))
#define UJSON_CHECK(t)   do { if (UJSON_TOKEN != (t)) { LOGE("%s:%d: Expected %s, got %s\n", __FILE__,__LINE__, \
	json_token(t), json_token(UJSON_TOKEN)); return UJSON_ERROR; } } while (0)


#define UJSON_OBJ_PROP(name, ...)                 \
	UJSON_MATCH(name) {                       \
		UJSON_ADVANCE();                  \
		UJSON_EXPECT(JSON_OBJECT_VALUE);  \
		do { __VA_ARGS__; } while (0);    \
		continue;                         \
	}


#define UJSON_OBJ_PROP_VAL(name, type, ...)     \
	UJSON_OBJ_PROP(name,                    \
		UJSON_CHECK(type);              \
		do { __VA_ARGS__; } while (0);  \
		UJSON_ADVANCE();                \
		continue;                       \
	)


#define UJSON_OBJ_ITER_BEGIN()                    \
	UJSON_EXPECT(JSON_OBJECT_BEGIN);          \
	while (UJSON_TOKEN == JSON_OBJECT_KEY) {  \
		UJSON_ADVANCE();                  \
		UJSON_CHECK(JSON_STRING);


#define UJSON_OBJ_ITER_END(rv)         \
	}                              \
	UJSON_CHECK(JSON_OBJECT_END);  \
	CO_YIELD(rv);


#define UJSON_ARRAY_ITER_BEGIN()               \
	UJSON_EXPECT(JSON_ARRAY_BEGIN);        \
	while (UJSON_TOKEN != JSON_ARRAY_END) {


#define UJSON_ARRAY_ITER_END()       \
	}                            \
	UJSON_EXPECT(JSON_ARRAY_END);


#define UJSON_OBJ_ITER(...)       UJSON_OBJ_ITER_BEGIN();   __VA_ARGS__; UJSON_OBJ_ITER_END(UJSON_OK)
#define UJSON_OBJ_ITER_EMIT(...)  UJSON_OBJ_ITER_BEGIN();   __VA_ARGS__; UJSON_OBJ_ITER_END(UJSON_EMIT)
#define UJSON_ARRAY_ITER(...)     UJSON_ARRAY_ITER_BEGIN(); __VA_ARGS__; UJSON_ARRAY_ITER_END()

#define UJSON_OBJ_PROP_I(name, ...)  UJSON_OBJ_PROP_VAL(name, JSON_NUMBER, __VA_ARGS__; LOGD("%s:%d: [JSON] " #name " = %d\n",   __FILE__, __LINE__, atoi(UJSON_STATE.str()));  )
#define UJSON_OBJ_PROP_F(name, ...)  UJSON_OBJ_PROP_VAL(name, JSON_NUMBER, __VA_ARGS__; LOGD("%s:%d: [JSON] " #name " = %f\n",   __FILE__, __LINE__, atof(UJSON_STATE.str()));  )
#define UJSON_OBJ_PROP_S(name, ...)  UJSON_OBJ_PROP_VAL(name, JSON_STRING, __VA_ARGS__; LOGD("%s:%d: [JSON] " #name " = '%s'\n", __FILE__, __LINE__, UJSON_STATE.str());        )
#define UJSON_OBJ_PROP_B(name, ...)  UJSON_OBJ_PROP_VAL(name, JSON_BOOL,   __VA_ARGS__; LOGD("%s:%d: [JSON] " #name " = %d\n",   __FILE__, __LINE__, atoi(UJSON_STATE.str()));  )
#define UJSON_OBJ_PROP_N(name, ...)  UJSON_OBJ_PROP_VAL(name, JSON_NULL,   __VA_ARGS__; LOGD("%s:%d: [JSON] " #name " = NULL\n", __FILE__, __LINE__); )


#endif


