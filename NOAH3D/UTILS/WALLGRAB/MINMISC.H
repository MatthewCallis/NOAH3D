
void	MS_Quit (char *error, ...);
int		MS_CheckParm (char far *check);

int SafeOpenWrite (char *filename);
int SafeOpenRead (char *filename);
void SafeRead (int handle, void far *buffer, unsigned count);
void SafeWrite (int handle, void far *buffer, unsigned count);
void far *SafeMalloc (long size);

void DefaultExtension (char *path, char *extension);
void DefaultPath (char *path, char *basepath);
void ExtractFileBase (char *path, char *dest);

