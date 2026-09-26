// stdafx.cpp : source file that includes just the standard includes
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
void convertPath(char* target, char const* source)
{
	char const* s;
	char *t;
	for (s=source, t=target; ((s-source)<MAX_PATH_LEN) && (*s!='\0'); s++, t++)
	{
		if (strchr(ALL_SPLITS,*s)!=NULL)
		{
			*t=PATH_SPLIT;
		}
		else
		{
			*t=*s;
		}
	}
	*t='\0';
}

FILE* mfopen(char const* filename, char const* mode)
{
	char actualName[MAX_PATH_LEN+1];
	convertPath(actualName,filename);
	return fopen(actualName,mode);
}
