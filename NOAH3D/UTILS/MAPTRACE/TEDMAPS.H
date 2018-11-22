//-------------------------------------------------------------------
// TEDMAPS.H
//

typedef	struct
{
	long		planestart[3];
	USHORT	planelength[3];
	USHORT	width,height;
	char		name[16];
} maptype;

extern maptype header;

void LoadTedHeader (const char *extension);
void LoadMap (int mapnum);


