#include "\miscsrc\types.h"

#define	MAXTOKEN	128

extern	char	token[MAXTOKEN];
extern	char	*scriptbuffer,*script_p,*scriptend_p;
extern	int		grabbed;
extern	int		scriptline;
extern	boolean	endofscript;


void LoadScriptFile (char *filename);
void GetToken (boolean crossline);
void UnGetToken (void);
boolean TokenAvailable (void);


