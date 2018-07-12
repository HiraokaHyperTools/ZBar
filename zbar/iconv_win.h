#pragma once

#include <stdint.h>

typedef uint32_t iconv_t;

#ifdef __cplusplus
extern "C"
#endif
iconv_t iconv_open(const char *tocode, const char *fromcode);

#ifdef __cplusplus
extern "C"
#endif
size_t iconv(iconv_t cd,
	char **inbuf, size_t *inbytesleft,
	char **outbuf, size_t *outbytesleft);

#ifdef __cplusplus
extern "C"
#endif
int iconv_close(iconv_t cd);
