
#include <Windows.h>
#include "ximage.h"
#include "cximage_for_zbar.h"

MagickWand *NewMagickWand()
{
	return new MagickWand();
}

bool MagickReadImage(MagickWand *wand, const char *filename)
{
	if (wand->image.Load(filename))
	{
		wand->image.AlphaDelete();

		if (wand->image.GrayScale())
		{
			return true;
		}
	}
	return false;
}

int MagickGetNumberImages(MagickWand *wand)
{
	int n = wand->image.GetNumFrames();
	if (n == 0)
	{
		n = 1;
	}
	return n;
}

bool MagickSetImageIndex(MagickWand *wand, int index)
{
	wand->image.SetFrame(index);
	return 1;
}

unsigned long MagickGetImageWidth(MagickWand *wand)
{
	return wand->image.GetWidth();
}

unsigned long MagickGetImageHeight(MagickWand *wand)
{
	return wand->image.GetHeight();
}

unsigned int MagickGetImagePixels(MagickWand *wand, const long x_offset, const long y_offset,
	const unsigned long columns, const unsigned long rows,
	const char *map, const StorageType storage,
	unsigned char *pixels)
{
	unsigned long destOff = 0;
	for (unsigned long y = 0; y < rows; y++)
	{
		const uint8_t *source = wand->image.GetBits(y + y_offset) + x_offset;
		memcpy(&pixels[destOff], source, columns);
		destOff += columns;
	}
	return rows;
}

void DestroyMagickWand(MagickWand *wand)
{
	delete wand;
}

void InitializeMagick(const char *argv0)
{

}

void DestroyMagick()
{

}

char *MagickGetException(const MagickWand *wand, ExceptionType *severity)
{
	*severity = WarningException;
	return nullptr;
}

void *MagickRelinquishMemory(void *resource)
{
	return nullptr;
}
