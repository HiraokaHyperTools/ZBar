
#include <Windows.h>
#include "iconv_win.h"

iconv_t iconv_open(const char *tocode, const char *fromcode)
{
	iconv_t converter = 0;
	if (stricmp(fromcode, "UTF-8") == 0)
	{
		converter |= CP_UTF8 << 16;
	}
	else if (stricmp(fromcode, "SJIS") == 0)
	{
		converter |= 932 << 16;
	}
	else
	{
		converter |= 819 << 16;
	}

	if (stricmp(tocode, "UTF-8") == 0)
	{
		converter |= CP_UTF8;
	}
	else if (stricmp(tocode, "SJIS") == 0)
	{
		converter |= 932;
	}
	else
	{
		converter |= 819;
	}

	return converter;
}

size_t iconv(iconv_t cd,
	char **inbuf, size_t *inbytesleft,
	char **outbuf, size_t *outbytesleft)
{
	wchar_t wideBuf[5000];
	int nRead = MultiByteToWideChar(cd >> 16, MB_ERR_INVALID_CHARS, *inbuf, *inbytesleft, wideBuf, 5000);
	if (nRead == 0)
	{
		return (size_t)-1;
	}
	int nWritten = WideCharToMultiByte(cd & 0xFFFFU, 0, wideBuf, nRead, *outbuf, *outbytesleft, NULL, NULL);
	if (nWritten == 0)
	{
		return (size_t)-1;
	}
	*inbuf = strchr(*inbuf, 0);
	*inbytesleft = 0;
	*outbuf += nWritten;
	*outbytesleft -= nWritten;
	return nWritten;
}

int iconv_close(iconv_t cd)
{
	return 0;
}
