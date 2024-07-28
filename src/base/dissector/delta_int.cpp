#include <base/dissector/color.h>
#include <base/dissector/byte_printer.h>
#include <base/dissector/item_printer.h>
#include <base/dissector/netobj_to_str.h>

#include <cstring>
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

#define SEP dbg_msg("network_in", "  * +---------+------+--------------------------+----------------------------------+---------------------------+");

void print_snap_int(int Index, const char *pDescription, int Value, int ItemNum, int MaxItems)
{
	CPacker Packer;
	Packer.Reset();
	Packer.AddInt(Value);
	char aHex[512];
	str_hex(aHex, sizeof(aHex), Packer.Data(), Packer.Size());
	char aItemNum[16];
	aItemNum[0] = '\0';
	if(ItemNum && MaxItems)
		str_format(aItemNum, sizeof(aItemNum), "%d/%d", ItemNum, MaxItems);
	int DescLen = 32 + count_color_code_len(pDescription);
	dbg_msg("network_in", "  * | %-7s | %-4d | %-24d | %-*s | %-25s |", aItemNum, Index, Value, DescLen, pDescription, aHex);
}

int snapshot_delta_intdump(const CSnapshot *pFrom, CSnapshot *pTo, const void *pSrcData, int DataSize, const short *ppItemSizes, bool Sixup)
{
	const CSnapshotDelta::CData *pDelta = (const CSnapshotDelta::CData *)pSrcData;
	const int *pData = (const int *)pDelta->m_aData;
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


	SEP;
	dbg_msg("network_in", "  * | %-7s | %-4s | %-24s | %-32s | %-25s |", "Item", "Idx", "Value", "Description", "Network raw (int packed)"); 
	SEP;


	int IntNum = 0;
	print_snap_int(IntNum++, "NumDeletedItems", pDelta->m_NumDeletedItems, 0, 0);
	print_snap_int(IntNum++, "NumUpdatedItems", pDelta->m_NumUpdateItems, 0, 0); 
	print_snap_int(IntNum++, "NumTempItems", pDelta->m_NumTempItems, 0, 0); 

	for(int i = 0; i < pDelta->m_NumDeletedItems; i++)
	{
		print_snap_int(IntNum++, "deleted key", pDeleted[i], 0, 0); 
	}


	// unpack updated stuff
	for(int i = 0; i < pDelta->m_NumUpdateItems; i++)
	{
		SEP;
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
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "ItemType (%s)", netobj_to_str(Type));
		print_snap_int(IntNum++, aBuf, Type, i + 1, pDelta->m_NumUpdateItems); 

		Id = *pData++;
		if(Id < 0 || Id > CSnapshot::MAX_ID)
		{
			dbg_msg("network_in", "    id=%d out of bounds", Id);
			return -5;
		}
		print_snap_int(IntNum++, "ItemId", Id, i + 1, pDelta->m_NumUpdateItems); 

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
			str_format(aBuf, sizeof(aBuf), "%s! ItemSize ! (0.7.X compat mode)%s", TERM_YELLOW, TERM_RESET);
			print_snap_int(IntNum++, aBuf, ItemSize, i + 1, pDelta->m_NumUpdateItems);
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
			print_snap_int(IntNum++, "item payload", pData[b], i + 1, pDelta->m_NumUpdateItems); 

		pData += ItemSize/4;

	}
	SEP;
	return 0;
}
