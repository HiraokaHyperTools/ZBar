
#include <Windows.h>
#include "ximage.h"
#include "cximage_for_zbar.h"

MagickWand *NewMagickWand()
{
	return new MagickWand();
}

bool MagickReadImage(MagickWand *wand, const char *filename)
{
	wand->filePath.assign(filename);
	return MagickSetImageIndex(wand, 0, 0);
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

bool MagickSetImageIndex(MagickWand *wand, int index, int super)
{
	wand->image.SetFrame(index);
	if (wand->image.Load(wand->filePath.c_str()))
	{
		wand->image.AlphaDelete();

		if (super & 4)
		{
			switch (wand->image.GetBpp()) {
			case 1:
				wand->image.Erode();
				break;
			default:
				wand->image.GaussianBlur();
				break;
			}
		}
		else
		{
			if (super & 1)
			{
				wand->image.GaussianBlur();
			}
			if (super & 2)
			{
				switch (wand->image.GetBpp()) {
				case 1:
					break;
				default:
					wand->image.Threshold(160);
					break;
				}

				wand->image.Erode();
			}
		}

		if (wand->image.GrayScale())
		{
			return true;
		}
		else
		{
			wand->exceptionType = FilterError;
		}
	}
	else
	{
		wand->exceptionType = CorruptImageError;
	}

	wand->exceptionMessage.assign(wand->image.GetLastError());
	return false;
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
	*severity = wand->exceptionType;
	return const_cast<char *>(wand->exceptionMessage.c_str());
}

void *MagickRelinquishMemory(void *resource)
{
	return nullptr;
}
