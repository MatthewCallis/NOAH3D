//
//
//

#include <stdio.h>
#include <stdlib.h>

typedef	enum
		{
			sc_Null,
			sc_NoteOn,
			sc_NoteOff,
			sc_PitchBend,
			sc_Instrument,
			sc_SongEnd,
			sc_PercOn,
			sc_Bad
		} SNESCmd;

	char *snames[sc_Bad] = {"NULL","ON  ","OFF ","BEND","INST","END ","PERC"};

void
doit(char *name)
{
	char vinst[32];

	for (int i = 0;i < 16;i++)
		vinst[i] = 0;

	char temp;
	FILE *f = fopen(name,"rb");
	if (!f)
	{
		printf("couldn't open %s for reading\n",name);
		exit(1);
	}
	printf("Dumping %s\n",name);
	fread(&temp,1,1,f);
	int count = temp;
	printf("%d instruments used:",count);
	int first = 1;
	while (count--)
	{
		if (!first)
			putchar(',');
		first = 0;
		fread(&temp,1,1,f);
		printf(" %3d",temp);
	}
	printf("\n");

	int done = 0;
	while (!done)
	{
		if (feof(f) || ferror(f))
		{
			puts("incorrect read error!");
			exit(1);
		}

		char cmd,voice,time;
		int time2 = 0;
		fread(&temp,1,1,f);
		cmd = temp >> 5;
		voice = (temp >> 2) & 7;
		time = temp & 3;
		printf("%d %s",voice,snames[cmd]);
		if (time == 3)
		{
			fread(&time2,1,1,f);
			if (!time2)
				time2 = 256;
			printf("-%3d ",time);
		}
		else
			printf("+%3d ",time2);

		switch (cmd)
		{
		case sc_NoteOn:
			if (vinst[voice] & 0x80)
			{
				fread(&temp,1,1,f);
				printf("P%3d",temp);
			}
			else
			{
				char note,vel;
				fread(&note,1,1,f);
				if (note & 0x80)
				{
					note &= 0x7f;
					fread(&vel,1,1,f);
					printf("M%3d+%3d",note,vel);
				}
				else
					printf("M%3d",note);
			}
			break;
		case sc_PitchBend:
			fread(&temp,1,1,f);
			int pba = temp;
			pba -= 0x80;
			printf("%3d",pba);
			break;
		case sc_Instrument:
			fread(&temp,1,1,f);
			printf("%3d",temp);
			vinst[voice] = temp;
			break;
		case sc_SongEnd:
			done = 1;
			break;
		}
/*
		case sc_NoteOn:
			if (vinst[s.voice] & 0x80)
			{
				fwrite(&s.data[1],1,1,f);	// Percussive with vel
#if DUMP
				printf("P%3d",s.data[1]);
#endif
			}
			else
			{
				if (s.data[1] == vvel[s.voice])
				{
					fwrite(s.data,1,1,f);	// Melodic, no vel
#if DUMP
					printf("M%3d",s.data[0]);
#endif
				}
				else
				{
					temp = s.data[0] | 0x80;
					fwrite(&temp,1,1,f);
					fwrite(&s.data[1],1,1,f);	// Melodic with vel
					vvel[s.voice] = s.data[1];
#if DUMP
					printf("M%3d+%3d",s.data[0],s.data[1]);
#endif
				}

			}
			break;
		case sc_PitchBend:
#if DUMP
			int pba = s.data[0];
			pba -= 0x80;
			printf("%3d",pba);
#endif
			fwrite(s.data,1,1,f);
			break;
		case sc_Instrument:
#if DUMP
			printf("%3d",s.data[0]);
#endif
			vinst[s.voice] = s.data[0];
			fwrite(s.data,1,1,f);
			break;
*/
		putchar('\n');
	}

	fclose(f);
}

int
main(int argc,char *argv[])
{
	if (argc != 2)
	{
		puts("Usage: dumpsnes FILE.SNM");
		return(1);
	}
	doit(argv[1]);
	return(0);
}
