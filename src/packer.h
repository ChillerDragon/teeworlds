#ifndef SANSIO_packer_H
#define SANSIO_packer_H

#include "enums.h"

/*
 * protocol: 0.6.5
 */
unsigned char *chunk_header_pack(unsigned char *pData, int Flags, int Size, int Sequence)
{
	pData[0] = ((Flags&3)<<6)|((Size>>4)&0x3f);
	pData[1] = (Size&0xf);
	if(Flags&NET_CHUNKFLAG_VITAL)
	{
		pData[1] |= (Sequence>>2)&0xf0;
		pData[2] = Sequence&0xff;
		return pData + 3;
	}
	return pData + 2;
}

/*
 * protocol: 0.6.5
 *
 * example:
 *
 *	int Flags, Size, Sequence;
 *	unsigned char aHeader[3] = {0x40, 0x10, 0x0a};
 *	chunk_header_unpack(aHeader, Flags, Size, Sequence);
 *
 *	printf("flags=%d size=%d sequence=%d\n", Flags, Size, Sequence);
 */
unsigned char *chunk_header_unpack(unsigned char *pData, int &Flags, int &Size, int &Sequence)
{
	Flags = (pData[0]>>6)&3;
	Size = ((pData[0]&0x3f)<<4) | (pData[1]&0xf);
	Sequence = -1;
	if(Flags&NET_CHUNKFLAG_VITAL)
	{
		Sequence = ((pData[1]&0xf0)<<2) | pData[2];
		return pData + 3;
	}
	return pData + 2;
}

#endif

