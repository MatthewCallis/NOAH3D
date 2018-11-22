#include <stdio.h>
#include <stdarg.h>
#include <process.h>
#include <dos.h>
#include <ctype.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <alloc.h>
#include "minmisc.h"
#pragma hdrstop


/*
=============================================================================

						MISC FUNCTIONS

=============================================================================
*/

/*
=================
=
= MS_Quit
=
= For abnormal program terminations
=
=================
*/

void MS_Quit (char *error, ...)
{
	va_list	argptr;

	if (error && *error)
	{
		va_start (argptr,error);
		vprintf (error,argptr);
		va_end (argptr);
		exit (1);
	}

	exit (1);
}


/*
=================
=
= MS_CheckParm
=
= Checks for the given parameter in the program's command line arguments
=
= Returns the argument number (1 to argc-1) or 0 if not present
=
=================
*/

int MS_CheckParm (char far *check)
{
	int		i;
	char	*parm;

	for (i = 1;i<_argc;i++)
	{
		parm = _argv[i];

		if ( !isalpha(*parm) )	// skip - / \ etc.. in front of parm
			if (!*++parm)
				continue;		// parm was only one char

		if ( !_fstricmp(check,parm) )
			return i;
	}

	return 0;
}




/*
=============================================================================

						   GENERIC FUNCTIONS

=============================================================================
*/

int SafeOpenWrite (char *filename)
{
	int	handle;

	handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_TRUNC
	, S_IREAD | S_IWRITE);

	if (handle == -1)
		MS_Quit ("Error opening %s: %s\n",filename,strerror(errno));

	return handle;
}

int SafeOpenRead (char *filename)
{
	int	handle;

	handle = open(filename,O_RDONLY | O_BINARY);

	if (handle == -1)
		MS_Quit ("Error opening %s: %s\n",filename,strerror(errno));

	return handle;
}


void SafeRead (int handle, void far *buffer, unsigned count)
{
	unsigned	iocount;

	_dos_read (handle,buffer,count,&iocount);
	if (iocount != count)
		MS_Quit ("File read failure\n");
}


void SafeWrite (int handle, void far *buffer, unsigned count)
{
	unsigned	iocount;

	_dos_write (handle,buffer,count,&iocount);
	if (iocount != count)
		MS_Quit ("File write failure\n");
}


void far *SafeMalloc (long size)
{
	void far *ptr;

	ptr = farmalloc (size);

	if (!ptr)
		MS_Quit ("Malloc failure for %lu bytes\n",size);

	return ptr;
}


void DefaultExtension (char *path, char *extension)
{
	char	*src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != '\\' && src != path)
	{
		if (*src == '.')
			return;			// it has an extension
		src--;
	}

	strcat (path, extension);
}

void DefaultPath (char *path, char *basepath)
{
	char	temp[128];

	if (path[0] == '\\')
		return;							// absolute path location
	strcpy (temp,path);
	strcpy (path,basepath);
	strcat (path,temp);
}


void ExtractFileBase (char *path, char *dest)
{
	char	*src;
	int		length;

	src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
	while (src != path && *(src-1) != '\\')
		src--;

//
// copy up to eight characters
//
	memset (dest,0,8);
	length = 0;
	while (*src && *src != '.')
	{
		if (++length == 9)
			MS_Quit ("Filename base of %s >8 chars",path);
		*dest++ = toupper(*src++);
	}
}


