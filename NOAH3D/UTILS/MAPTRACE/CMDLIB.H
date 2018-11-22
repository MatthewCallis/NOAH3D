// cmdlib.h

#ifndef __CMDLIB__
#define __CMDLIB__

typedef unsigned char 		byte;
typedef unsigned short		USHORT;
#define	ushort	USHORT
typedef unsigned long		ULONG;

typedef enum {false,true} boolean;

extern	int	_argc;
extern	char	**_argv;

extern	boolean	graphicson;

long	filelength (int handle);
long	tell (int handle);

void	Error (char *error, ...);
int	CheckParm (char *check);

int 	SafeOpenWrite (char *filename);
int 	SafeOpenRead (char *filename);
void 	SafeRead (int handle, void *buffer, long count);
void 	SafeWrite (int handle, void *buffer, long count);
void 	*SafeMalloc (long size);

long	LoadFile (char *filename, void **bufferptr);
void	SaveFile (char *filename, void *buffer, long count);

void 	DefaultExtension (char *path, char *extension);
void 	DefaultPath (char *path, char *basepath);
void 	ExtractFileBase (char *path, char *dest);

long 	ParseNum (char *str);

short	MotoShort (short l);
short	IntelShort (short l);
long	MotoLong (long l);
long	IntelLong (long l);

void 	GetPalette (byte *pal);
void 	SetPalette (byte *pal);
void 	VGAMode (void);
void 	TextMode (void);


#endif
