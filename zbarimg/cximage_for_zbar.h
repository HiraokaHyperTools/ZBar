
#pragma once

typedef enum {
	UndefinedException, WarningException = 300, ResourceLimitWarning = 300, TypeWarning = 305,
	OptionWarning = 310, DelegateWarning = 315, MissingDelegateWarning = 320, CorruptImageWarning = 325,
	FileOpenWarning = 330, BlobWarning = 335, StreamWarning = 340, CacheWarning = 345,
	CoderWarning = 350, FilterWarning = 352, ModuleWarning = 355, DrawWarning = 360,
	ImageWarning = 365, WandWarning = 370, RandomWarning = 375, XServerWarning = 380,
	MonitorWarning = 385, RegistryWarning = 390, ConfigureWarning = 395, PolicyWarning = 399,
	ErrorException = 400, ResourceLimitError = 400, TypeError = 405, OptionError = 410,
	DelegateError = 415, MissingDelegateError = 420, CorruptImageError = 425, FileOpenError = 430,
	BlobError = 435, StreamError = 440, CacheError = 445, CoderError = 450,
	FilterError = 452, ModuleError = 455, DrawError = 460, ImageError = 465,
	WandError = 470, RandomError = 475, XServerError = 480, MonitorError = 485,
	RegistryError = 490, ConfigureError = 495, PolicyError = 499, FatalErrorException = 700,
	ResourceLimitFatalError = 700, TypeFatalError = 705, OptionFatalError = 710, DelegateFatalError = 715,
	MissingDelegateFatalError = 720, CorruptImageFatalError = 725, FileOpenFatalError = 730, BlobFatalError = 735,
	StreamFatalError = 740, CacheFatalError = 745, CoderFatalError = 750, FilterFatalError = 752,
	ModuleFatalError = 755, DrawFatalError = 760, ImageFatalError = 765, WandFatalError = 770,
	RandomFatalError = 775, XServerFatalError = 780, MonitorFatalError = 785, RegistryFatalError = 790,
	ConfigureFatalError = 795, PolicyFatalError = 799
} ExceptionType;

typedef enum
{
	CharPixel,
	ShortPixel,
	IntegerPixel,
	LongPixel,
	FloatPixel,
	DoublePixel
} StorageType;

#ifdef __cplusplus
# define EXTERN_C extern "C"
# include <string>
class MagickWand
{
public:
	//! image
	CxImage image;

	//! filepath
	std::string filePath;

	//! error type
	ExceptionType exceptionType = UndefinedException;

	//! error message
	std::string exceptionMessage;

	//! dtor
	virtual ~MagickWand()
	{

	}
};
#else
# include <stdint.h>
# define EXTERN_C
typedef uint8_t bool;
typedef struct
{
	int dummy;
} MagickWand;
#endif

EXTERN_C
MagickWand *NewMagickWand();
EXTERN_C
bool MagickReadImage(MagickWand *wand, const char *filename);
EXTERN_C
int MagickGetNumberImages(MagickWand *wand);
EXTERN_C
bool MagickSetImageIndex(MagickWand *wand, int index, int super);
EXTERN_C
unsigned long MagickGetImageWidth(MagickWand *wand);
EXTERN_C
unsigned long MagickGetImageHeight(MagickWand *wand);
EXTERN_C
unsigned int MagickGetImagePixels(MagickWand *wand, const long x_offset, const long y_offset,
	const unsigned long columns, const unsigned long rows,
	const char *map, const StorageType storage,
	unsigned char *pixels);
EXTERN_C
void DestroyMagickWand(MagickWand *wand);
EXTERN_C
void InitializeMagick(const char *argv0);
EXTERN_C
void DestroyMagick();
EXTERN_C
char *MagickGetException(const MagickWand *wand, ExceptionType *severity);
EXTERN_C
void *MagickRelinquishMemory(void *resource);
