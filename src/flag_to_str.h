#ifndef SANSIO_FLAG_TO_STR_H
#define SANSIO_FLAG_TO_STR_H

#include <string.h>

#include "enums.h"

/*
* Given a Flags integer it will fill a buffer with
* the flags represented as string
* all possible string values:
*	0 => ""
*	1 => "NET_CHUNKFLAG_VITAL"
*	2 => "NET_CHUNKFLAG_RESEND"
*	3 => "NET_CHUNKFLAG_VITAL | NET_CHUNKFLAG_RESEND"
*
* Example usage:
*
	int Flag = 3;
	char aBuf[512];
	flag_chunk_to_str(Flag, aBuf, sizeof(aBuf));
	printf("%s\n", aBuf);
*/
void flag_chunk_to_str(int Flags, char *pBuf, int BufSize)
{
	pBuf[0] = '\0';
	if(Flags&NET_CHUNKFLAG_VITAL)
		strncat(pBuf, "NET_CHUNKFLAG_VITAL", BufSize);
	if(Flags&NET_CHUNKFLAG_RESEND)
	{
		if(pBuf[0])
			strncat(pBuf, " | NET_CHUNKFLAG_RESEND", BufSize);
		else
			strncat(pBuf, "NET_CHUNKFLAG_RESEND", BufSize);
	}
	strcat(pBuf, "\0");
	pBuf[BufSize] = '\0';
}

#endif

