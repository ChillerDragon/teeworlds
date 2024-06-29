#include <base/dissector/color.h>
#include <base/dissector/byte_printer.h>
#include <base/dissector/item_printer.h>

#include <engine/shared/packer.h>
#include <engine/shared/network.h>
#include <base/math.h>
#include <engine/shared/config.h>

#include <stdarg.h>

#include <base/math.h>
#include <base/system.h>

#include <engine/client.h>
#include <engine/config.h>
#include <engine/console.h>
#include <engine/editor.h>
#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/map.h>

// #if __has_include("dissector.h") && __has_include(<engine/masterserver.h>)

#if __has_include(<engine/masterserver.h>)
#include <engine/masterserver.h>
#endif

#include <engine/serverbrowser.h>
#include <engine/sound.h>
#include <engine/storage.h>
#include <engine/textrender.h>

#include <engine/shared/config.h>
#include <engine/shared/compression.h>
#include <engine/shared/datafile.h>
#include <engine/shared/demo.h>
#include <engine/shared/filecollection.h>
#if __has_include(<engine/shared/mapchecker.h>)
#include <engine/shared/mapchecker.h>
#endif
#include <engine/shared/network.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>
#include <engine/shared/ringbuffer.h>
#include <engine/shared/snapshot.h>

#include <game/version.h>

#if __has_include(<mastersrv/mastersrv.h>)
#include <mastersrv/mastersrv.h>
#endif
#if __has_include(<versionsrv/versionsrv.h>)
#include <versionsrv/versionsrv.h>
#endif

#if __has_include(<engine/contacts.h>)
#include <engine/contacts.h>
#endif
#include <engine/serverbrowser.h>
#include <engine/client/serverbrowser.h>
#if __has_include(<engine/client/contacts.h>)
#include <engine/client/contacts.h>
#endif
#include <engine/client.h>
#include <engine/client/client.h>

#if __has_include(<generated/protocol.h>)
#include <generated/protocol.h>
#endif
#include <limits.h>

#include "compat.h"

#include "snapshot.h"


#include "delta_int.h"

// TODO: remove or share with snapshot.h
static int _RangeCheck(const void *pEnd, const void *pPtr, int Size)
{
	if((const char *)pPtr + Size > (const char *)pEnd)
		return -1;
	return 0;
}

int snapshot_delta_intdump(const CSnapshot *pFrom, CSnapshot *pTo, const void *pSrcData, int DataSize, const short *ppItemSizes, bool Sixup)
{
	const CSnapshotDelta::CData *pDelta = (const CSnapshotDelta::CData *)pSrcData;
#ifdef _PROTOCOL_VERSION_7
	const int *pData = (const int *)pDelta->m_pData;
#else
	const int *pData = (const int *)pDelta->m_aData;
#endif
	const int *pEnd = (const int *)(((const char *)pSrcData + DataSize));

	int ItemSize;
	const int *pDeleted;
	int Id, Type;

	if(!Sixup)
	{
		dbg_msg("network_in", "  Snapshot info only supported for 0.7");
		return -99;
	}
	dbg_msg("network_in", "  *****************************");
	dbg_msg("network_in", "  * dumping decompressed snapshot ints. All net ints are already unpacked.");
	dbg_msg("network_in", "  * so this is not what is sent over the network.");

	// unpack deleted stuff
	pDeleted = pData;
	if(pDelta->m_NumDeletedItems < 0)
		return -1;

	pData += pDelta->m_NumDeletedItems;
	if(pData > pEnd)
		return -2;

	// 	int m_NumDeletedItems;
	// 	int m_NumUpdateItems;
	// 	int m_NumTempItems; // needed?
	// 	int m_pData[1];

	int IntNum = 0;
	dbg_msg("network_in", "  * UnpackedInt[%d] = %d; // NumDeletedItems", IntNum++, pDelta->m_NumDeletedItems); 
	dbg_msg("network_in", "  * UnpackedInt[%d] = %d; // NumUpdatedItems", IntNum++, pDelta->m_NumUpdateItems); 
	dbg_msg("network_in", "  * UnpackedInt[%d] = %d; // NumTempItems", IntNum++, pDelta->m_NumTempItems); 

	for(int i = 0; i < pDelta->m_NumDeletedItems; i++)
	{
		dbg_msg("network_in", "  * UnpackedInt[%d] = %d; // deleted key", IntNum++, pDeleted[i]); 
	}

	// unpack updated stuff
	for(int i = 0; i < pDelta->m_NumUpdateItems; i++)
	{
		dbg_msg("network_in", "  ** item %d/%d", i + 1, pDelta->m_NumUpdateItems);
		if(pData+2 > pEnd)
		{
			dbg_msg("network_in", "    reading past end");
			return -3;
		}

		Type = *pData++;
		if(Type < 0 || Type > CSnapshot::MAX_TYPE)
		{
			dbg_msg("network_in", "    type=%d out of bounds", Type);
			return -4;
		}
		dbg_msg("network_in", "  * UnpackedInt[%d] = %d; // ItemType", IntNum++, Type); 

		Id = *pData++;
		if(Id < 0 || Id > CSnapshot::MAX_ID)
		{
			dbg_msg("network_in", "    id=%d out of bounds", Id);
			return -5;
		}
		dbg_msg("network_in", "  * UnpackedInt[%d] = %d; // ItemId", IntNum++, Id); 

		if(Type < _MAX_NETOBJSIZES && ppItemSizes[Type])
			ItemSize = ppItemSizes[Type];
		else
		{
			if(pData+1 > pEnd)
			{
				dbg_msg("network_in", "    reading past end 2");
				return -6;
			}
			if(*pData < 0 || *pData > INT_MAX / 4)
			{
				dbg_msg("network_in", "    int out of bounds");
				return -7;
			}
			ItemSize = (*pData++) * 4;
			dbg_msg("network_in", "  * UnpackedInt[%d] = %d; // ItemSize", IntNum++, ItemSize); 
		}

		if(ItemSize < 0 || _RangeCheck(pEnd, pData, ItemSize))
		{
			if(ItemSize < 0)
				dbg_msg("network_in", "  ItemSize=%d (should not be negative)", ItemSize);
			else
			{
				char aHexData[512];
				str_hex(aHexData, sizeof(aHexData), pData, ItemSize);
				dbg_msg("network_in", "  RangeCheck(ItemSize=%d) failed", ItemSize);
				print_raw("network_in", "    pData: ", pData, ItemSize);
				print_raw("network_in", "    pEnd: ", pEnd, ItemSize);
				dbg_msg("network_in", "    (const char *)pData + Size > (const char *)pEnd");
				dbg_msg("network_in", "    %s + %d > %s", (const char *)pData, ItemSize, (const char *)pEnd);
				dbg_msg("network_in", "    %s > %s", (const char *)pData + ItemSize, (const char *)pEnd);
				// dbg_msg("network_in", "    %x > %x", (const char *)pData + ItemSize, (const char *)pEnd);
			}
		}

		for(int b = 0; b < ItemSize/4; b++)
			dbg_msg("network_in", "  * UnpackedInt[%d] = %d; // item payload", IntNum++, pData[b]); 

		pData += ItemSize/4;

		dbg_msg("network_in", "  **");
	}
	return 0;
}
