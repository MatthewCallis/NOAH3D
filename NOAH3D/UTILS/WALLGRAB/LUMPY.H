#include "types.h"
#include "minmisc.h"
#include "script.h"

#define SCREENWIDTH		320
#define SCREENHEIGHT	200

#define PEL_WRITE_ADR		0x3c8
#define PEL_READ_ADR		0x3c7
#define PEL_DATA			0x3c9


extern	boolean	pause;
extern	byte	*lumpbuffer,*lump_p;

void LoadLBM (char *filename);

