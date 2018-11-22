#include <io.h>
#include <dos.h>
#include <fcntl.h>
#include <alloc.h>

#include "script.h"
#include "minmisc.h"
#pragma hdrstop

/*
=============================================================================

						PARSING STUFF

=============================================================================
*/

char	token[MAXTOKEN];
char	*scriptbuffer,*script_p,*scriptend_p;
int		grabbed;
int		scriptline;
boolean	endofscript;
boolean	tokenready;			// only true if UnGetToken was just called

/*
==============
=
= LoadScriptFile
=
==============
*/

void LoadScriptFile (char *filename)
{
	int			handle;
	unsigned	iocount;
	long		size;

	handle = _open (filename,O_RDONLY);
	if ( handle == -1)
		MS_Quit ("Cannot open %s\n",filename);

	size = filelength (handle);
	scriptbuffer = malloc (size);
	if (!scriptbuffer)
		MS_Quit ("Cannot allocate script buffer\n");

	_dos_read (handle,scriptbuffer,size,&iocount);
	if (iocount != size)
		MS_Quit ("Read failure on %s\n",filename);

	close (handle);

	script_p = scriptbuffer;
	scriptend_p = script_p + size;
	scriptline = 1;
	endofscript = false;
	tokenready = false;
}


/*
==============
=
= UnGetToken
=
= Signals that the current token was not used, and should be reported
= for the next GetToken.  Note that

GetToken (true);
UnGetToken ();
GetToken (false);

= could cross a line boundary.
=
==============
*/

void UnGetToken (void)
{
	tokenready = true;
}


/*
==============
=
= GetToken
=
==============
*/

void GetToken (boolean crossline)
{
char	*token_p;

	if (tokenready)				// is a token allready waiting?
	{
		tokenready = false;
		return;
	}

	if (script_p >= scriptend_p)
	{
		if (!crossline)
			MS_Quit ("Line %i is incomplete\n",scriptline);
		endofscript = true;
		return;
	}

//
// skip space
//
skipspace:
	while (*script_p <= 32)
	{
		if (script_p >= scriptend_p)
		{
			if (!crossline)
				MS_Quit ("Line %i is incomplete\n",scriptline);
			endofscript = true;
			return;
		}
		if (*script_p++ == '\n')
		{
			if (!crossline)
				MS_Quit ("Line %i is incomplete\n",scriptline);
			scriptline++;
		}
	}

	if (script_p >= scriptend_p)
	{
		if (!crossline)
			MS_Quit ("Line %i is incomplete\n",scriptline);
		endofscript = true;
		return;
	}

	if (*script_p == ';')	// semicolon is comment field
	{
		if (!crossline)
			MS_Quit ("Line %i is incomplete\n",scriptline);
		while (*script_p++ != '\n')
			if (script_p >= scriptend_p)
			{
				endofscript = true;
				return;
			}
		goto skipspace;
	}

//
// copy token
//
	token_p = token;

	while ( *script_p > 32 && *script_p != ';')
	{
		*token_p++ = *script_p++;
		if (script_p == scriptend_p)
			break;
		if (token_p == &token[MAXTOKEN])
			MS_Quit ("Token too large on line %i\n",scriptline);
	}

	*token_p = 0;
}


/*
==============
=
= TokenAvailable
=
= Returns true if there is another token on the line
=
==============
*/

boolean TokenAvailable (void)
{
	char	*search_p;

	search_p = script_p;

	if (search_p >= scriptend_p)
		return false;

	while ( *search_p <= 32)
	{
		if (*search_p == '\n')
			return false;
		search_p++;
		if (search_p == scriptend_p)
			return false;

	}

	if (*search_p == ';')
		return false;

	return true;
}


