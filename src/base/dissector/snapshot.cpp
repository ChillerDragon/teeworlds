// ChillerDragon 2022 - dissector

#include <base/dissector/color.h>
#include <base/dissector/byte_printer.h>
#include <base/dissector/item_printer.h>
#include <base/dissector/delta_int.h>
#include <base/dissector/netobj_to_str.h>

#include "dissector.h"

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

static int _RangeCheck(const void *pEnd, const void *pPtr, int Size)
{
	if((const char *)pPtr + Size > (const char *)pEnd)
		return -1;
	return 0;
}

void dump_snapshot_data(const void *pData, int Size)
{
	char aHex[128];
	str_hex(aHex, sizeof(aHex), pData, Size);
	char aBin[128];
	str_bin(aBin, sizeof(aBin), pData, Size);
	dbg_msg("item", "%s | %s", aHex, aBin);
}

bool dump_snap_item7(const CSnapshotItem *pItem, int Size)
{
#ifdef _PROTOCOL_VERSION_7
	int b = 0;
	if(pItem->Type() == NETOBJTYPE_PLAYERINFO)
	{
		if(Size != 12)
		{
			dbg_msg("snapshot", "\t\tInvalid Size=%d Expected=12 Type=%d", Size, pItem->Type());
			dbg_break();
		}
		const CNetObj_PlayerInfo *pInfo = ((const CNetObj_PlayerInfo *)pItem->Data());

		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_PlayerFlags=%d", b, pItem->Data()[b], pItem->Data()[b], pInfo->m_PlayerFlags);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Score=%d", b, pItem->Data()[b], pItem->Data()[b], pInfo->m_Score);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Latency=%d", b, pItem->Data()[b], pItem->Data()[b], pInfo->m_Latency);b++;
	}
	else if(pItem->Type() == NETOBJTYPE_GAMEDATA)
	{
		if(Size != 12)
		{
			dbg_msg("snapshot", "\t\tInvalid Size=%d Expected=12 XType=%d", Size, pItem->Type());
			dbg_break();
		}
		const CNetObj_GameData *pGameData = ((const CNetObj_GameData *)pItem->Data());

		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_GameStartTick=%d", b, pItem->Data()[b], pItem->Data()[b], pGameData->m_GameStartTick);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_GameStateFlags=%d", b, pItem->Data()[b], pItem->Data()[b], pGameData->m_GameStateFlags);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_GameStateEndTick=%d", b, pItem->Data()[b], pItem->Data()[b], pGameData->m_GameStateEndTick);b++;
	}
	else if(pItem->Type() == NETOBJTYPE_CHARACTER)
	{
		if(Size != 88)
		{
			dbg_msg("snapshot", "\t\tInvalid Size=%d Expected=88 Type=%d", Size, pItem->Type());
			exit(1);
		}
		const CNetObj_Character *pChr = ((const CNetObj_Character *)pItem->Data());

		// dump_snapshot_data(pItem->Data(), Size);

		// CNetObj_CharacterCore
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Tick=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Tick);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_X=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_X);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Y=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Y);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_VelX=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_VelX);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_VelY=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_VelY);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Angle=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Angle);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Direction=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Direction);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Jumped=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Jumped);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_HookedPlayer=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_HookedPlayer);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_HookState=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_HookState);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_HookTick=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_HookTick);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_HookX=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_HookX);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_HookY=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_HookY);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_HookDx=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_HookDx);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_HookDy=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_HookDy);b++;

		// CNetObj_Character
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Health=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Health);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Armor=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Armor);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_AmmoCount=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_AmmoCount);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Weapon=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Weapon);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Emote=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Emote);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_AttackTick=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_AttackTick);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_TriggeredEvents=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_TriggeredEvents);b++;
	}
	else
	{
		return false;
	}
	return true;
#endif
	return false;
}

bool dump_snap_item6(const CSnapshotItem *pItem, int Size)
{
#ifdef _PROTOCOL_VERSION_6
	int b = 0;
	if(pItem->Type() == NETOBJTYPE_PLAYERINFO)
	{
		constexpr int ExpectedSize = 5 * 4;
		if(Size != ExpectedSize)
		{
			dbg_msg("snapshot", "\t\tInvalid Size=%d Expected=%d Type=%d", Size, ExpectedSize, pItem->Type());
			dbg_break();
		}
		const CNetObj_PlayerInfo *pInfo = ((const CNetObj_PlayerInfo *)pItem->Data());

		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Local=%d", b, pItem->Data()[b], pItem->Data()[b], pInfo->m_Local);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_ClientID=%d", b, pItem->Data()[b], pItem->Data()[b], pInfo->m_ClientID);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Team=%d", b, pItem->Data()[b], pItem->Data()[b], pInfo->m_Team);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Score=%d", b, pItem->Data()[b], pItem->Data()[b], pInfo->m_Score);b++;
		dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Latency=%d", b, pItem->Data()[b], pItem->Data()[b], pInfo->m_Latency);b++;
	}
	else
	{
		return false;
	}
	return true;
#endif
	return false;
}

void debug_dump(CSnapshot *pSnapShot)
{
	dbg_msg("snapshot", "num_items=%d", pSnapShot->NumItems());
	for(int i = 0; i < pSnapShot->NumItems(); i++)
	{
		const CSnapshotItem *pItem = pSnapShot->GetItem(i);
		int Size = pSnapShot->GetItemSize(i);
		dbg_msg("snapshot", "\ttype=%d (%s)  id=%d", pItem->Type(), netobj_to_str(pItem->Type()), pItem->ID());

#ifdef _PROTOCOL_VERSION_6
#else
		if(dump_snap_item7(pItem, Size))
			return;
#endif


// #ifdef _PROTOCOL_VERSION_6
// 			const protocol7::CNetObj_PlayerInfo *pInfo = ((const protocol7::CNetObj_PlayerInfo *)pItem->Data());
// #else
// 			const CNetObj_PlayerInfo *pInfo = ((const CNetObj_PlayerInfo *)pItem->Data());
// #endif

		for(int b = 0; b < Size / 4; b++)
			dbg_msg("snapshot", "\t\t%3d %12d\t%08x", b, pItem->Data()[b], pItem->Data()[b]);
	}
}

static void UndiffItem(const int *pPast, const int *pDiff, int *pOut, int Size)
{
	while(Size)
	{
		*pOut = *pPast+*pDiff;
		pOut++;
		pPast++;
		pDiff++;
		Size--;
	}
}

int CSnapshotDelta_UnpackDelta(const CSnapshot *pFrom, CSnapshot *pTo, const void *pSrcData, int DataSize, const short *ppItemSizes, bool Sixup)
{
	snapshot_delta_intdump(pFrom, pTo, pSrcData, DataSize, ppItemSizes, Sixup);

	CSnapshotBuilder Builder;
	const CSnapshotDelta::CData *pDelta = (const CSnapshotDelta::CData *)pSrcData;
#ifdef _PROTOCOL_VERSION_7
	const int *pData = (const int *)pDelta->m_pData;
#else
	const int *pData = (const int *)pDelta->m_aData;
#endif
	const int *pEnd = (const int *)(((const char *)pSrcData + DataSize));

	const CSnapshotItem *pFromItem;
	int Keep, ItemSize;
	const int *pDeleted;
	int Id, Type, Key;
	int FromIndex;
	int *pNewData;

	Builder.Init();

	dbg_msg("network_in", "  this is the new snapshot items we got in this snap. it contains only the changed items.");
	dbg_msg("network_in", "  CSnapshotDelta_UnpackDelta unpacking %d items:", pDelta->m_NumUpdateItems);
	if(!Sixup)
	{
		dbg_msg("network_in", "  Snapshot info only supported for 0.7");
		return -99;
	}
	int DataPrintSize = minimum(DataSize, 24);

	// unpack deleted stuff
	pDeleted = pData;
	if(pDelta->m_NumDeletedItems < 0)
	{
		dbg_msg("network_in", "    failed to unpack too few deletedItems=%d", pDelta->m_NumDeletedItems);
		return -1;
	}

	// char aCutNote[512];
	// aCutNote[0] = '\0';
	// if(DataPrintSize != DataSize)
	// 	str_format(aCutNote, sizeof(aCutNote), "[CUT OFF %d BYTES]", DataSize - DataPrintSize);

	if(pDelta->m_NumDeletedItems)
		print_hex_row_highlighted("network_in", "    ", pData, DataPrintSize, 0, pDelta->m_NumDeletedItems, "deleted items (%d)", pDelta->m_NumDeletedItems);

	pData += pDelta->m_NumDeletedItems;
	if(pData > pEnd)
	{
		dbg_msg("network_in", "    failed to unpack too many deletedItems=%d", pDelta->m_NumDeletedItems);
		return -2;
	}

	// copy all non deleted stuff
	for(int i = 0; i < pFrom->NumItems(); i++)
	{
		// dbg_assert(0, "fail!");
		pFromItem = pFrom->GetItem(i);
		ItemSize = pFrom->GetItemSize(i);
		Keep = 1;
		for(int d = 0; d < pDelta->m_NumDeletedItems; d++)
		{
			if(pDeleted[d] == pFromItem->Key())
			{
				Keep = 0;
				break;
			}
		}

		if(Keep)
		{
			// keep it
			mem_copy(
				Builder.NewItem(pFromItem->Type(), pFromItem->ID(), ItemSize),
				pFromItem->Data(), ItemSize);
		}
	}

	// unpack updated stuff
	for(int i = 0; i < pDelta->m_NumUpdateItems; i++)
	{
		const int *pItemData = pData;
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

		Id = *pData++;
		if(Id < 0 || Id > CSnapshot::MAX_ID)
		{
			dbg_msg("network_in", "    id=%d out of bounds", Id);
			return -5;
		}

		bool KnownSize = false;
		if(Type < _MAX_NETOBJSIZES && ppItemSizes[Type])
		{
			ItemSize = ppItemSizes[Type];
			KnownSize = true;
		}
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

		// if(RangeCheck(pEnd, pData, ItemSize) || ItemSize < 0) return -3;

		Key = (Type<<16)|(Id&0xffff);

		// create the item if needed
		pNewData = Builder.GetItemData(Key);
		if(!pNewData)
			pNewData = (int *)Builder.NewItem(Key>>16, Key&0xffff, ItemSize);

		//if(range_check(pEnd, pNewData, ItemSize)) return -4;

		FromIndex = pFrom->GetItemIndex(Key);
		if(FromIndex != -1)
		{
			// we got an update so we need to apply the diff
			UndiffItem(pFrom->GetItem(FromIndex)->Data(), pData, pNewData, ItemSize / 4);
			// m_aSnapshotDataUpdates[Type]++;
		}
		else // no previous, just copy the pData
		{
			mem_copy(pNewData, pData, ItemSize);
			// m_aSnapshotDataRate[Type] += ItemSize * 8;
			// m_aSnapshotDataUpdates[Type]++;
		}

		pData += ItemSize/4;

		dbg_msg("network_in", "  ------------------------------------------------");
		dbg_msg("network_in", "    UpdatedDeltaItem %d/%d of size=%d (in bytes)", i + 1, pDelta->m_NumUpdateItems, ItemSize);
		dbg_msg("network_in", " ");
		dbg_msg("network_in", "    Unpacked integers as raw bytes. This is the snap payload after huffman decompress AND int unpack");
		dbg_msg("network_in", "    vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv");
		int FullItemSize = ItemSize + (3 - (int)KnownSize) * sizeof(int);
		int PrintItemLen = minimum(20, FullItemSize);
		char aCutNote[512];
		char aTypeNote[512];
		char aIdNote[512];
		str_format(aTypeNote, sizeof(aTypeNote), "Type=%d (%s)", Type, netobj_to_str(Type));
		str_format(aIdNote, sizeof(aIdNote), "ID=%d", Id);
		aCutNote[0] = '\0';
		if(FullItemSize != PrintItemLen)
			str_format(aCutNote, sizeof(aCutNote), "[CUT OFF %d BYTES]", FullItemSize - PrintItemLen);
		if(KnownSize)
		{
			print_hex_row_highlight_two(
				"network_in",
				"    ",
				pItemData, PrintItemLen,
				0, 3,
				aTypeNote,
				4, 7,
				aIdNote,
				aCutNote);
		}
		else
		{
			char aSizeNote[512];
			str_format(aSizeNote, sizeof(aSizeNote), "Size: %d fields(ints) %d bytes", ItemSize / (int)sizeof(int), ItemSize);
			print_hex_row_highlight_three(
				"network_in",
				"    ",
				pItemData, PrintItemLen,
				0, 3,
				aTypeNote,
				4, 7,
				aIdNote,
				8, 11,
				aSizeNote,
				aCutNote);
		}
		// char aHex[512];
		// str_hex(aHex, sizeof(aHex), pItemData, FullItemSize);
		// dbg_msg("network_in", "     sizeknown=%d pItemData=%s", KnownSize, aHex);
		print_netobj_as_struct(pItemData, "    ");
	}

	dbg_msg("network_in", "finish up snap build items %d", Builder.NumItems());

	// finish up
	return Builder.Finish(pTo);
}


void print_snapshot(int Msg,
    CUnpacker &Unpacker,
    const CConfig *pConfig,
    int m_ReceivedSnapshots,
    unsigned int m_SnapshotParts,
    const CSnapshotDelta &m_SnapshotDelta,
    int m_CurrentRecvTick,
#ifdef _PROTOCOL_VERSION_7
    const CSnapshotStorage &m_SnapshotStorage,
#else
    CSnapshotStorage &m_SnapshotStorage,
#endif
    int m_SnapCrcErrors,
    // const CSmoothTime &GameTime,
    const char *m_aSnapshotIncomingData,
    CSnapshotStorage::CHolder **m_aSnapshots,
    CClient *pClient,
	bool Sixup)
{
	// either debug 3 or higher
	// or dbg_snap 1 or higher to see full snap dumps
	if(pConfig->m_Debug < 2 && pConfig->m_DbgSnap < 1)
		return;

	init_compat();

    // CSmoothTime m_GameTime = GameTime;
    // CSmoothTime m_GameTime; // TODO

	int NumParts = 1;
	int Part = 0;
	int GameTick = Unpacker.GetInt();
	int DeltaTick = GameTick-Unpacker.GetInt();
	int PartSize = 0;
	int Crc = 0;
	int CompleteSize = 0;
	const char *pData = 0;
	const char *pMsg = "unkown";
	if(Msg == NETMSG_SNAP)
		pMsg = "NETMSG_SNAP";
	if(Msg == NETMSG_SNAPSINGLE)
		pMsg = "NETMSG_SNAPSINGLE";
	if(Msg == NETMSG_SNAPEMPTY)
		pMsg = "NETMSG_SNAPEMPTY";

	if(Msg != NETMSG_SNAPEMPTY || pConfig->m_DbgSnap > 1)
	{
		dbg_msg("network_in", "SnapShot Msg=%d (%s) receivedSnaps=%d", Msg, pMsg, m_ReceivedSnapshots);
		dbg_msg("network_in", "  gametick=%d", GameTick);
		dbg_msg("network_in", "  deltatick=%d", DeltaTick);
	}

	// we are not allowed to process snapshot yet
	if(pClient->State() < IClient::STATE_LOADING)
	{
		dbg_msg("network_in", "  abort snapshot we are still in state loading");
		return;
	}

	if(Msg == NETMSG_SNAP)
	{
		NumParts = Unpacker.GetInt();
		Part = Unpacker.GetInt();
		dbg_msg("network_in", "  NETMSG_SNAP NumParts=%d Part=%d", NumParts, Part);
	}

	if(Msg != NETMSG_SNAPEMPTY)
	{
		Crc = Unpacker.GetInt();
		PartSize = Unpacker.GetInt();
		dbg_msg("network_in", "  Crc=%d PartSize=%d", Crc, PartSize);
	}

	// empty part sizes are expected and should be ignored silently
	// because they happen frequently
	// without this check we would trip the unpacker because it can not unpack
	// GetRaw(Size=0)
	if(PartSize == 0 && Msg == NETMSG_SNAPEMPTY)
		return;
	if(PartSize != 0 && Msg == NETMSG_SNAPEMPTY)
	{
		dbg_msg("network_in", "  got weird snapshot:");
		dbg_msg("network_in", "    snap empty with PartSize=%d (expected PartSize=0)", PartSize);
	}

	pData = (const char *)Unpacker.GetRaw(PartSize);


	if(Unpacker.Error())
	{
		dbg_msg("network_in", "  failed to unpack snapshot:");
		dbg_msg("network_in", "    Unpacker.GetRaw(PartSize=%d) => %p", PartSize, pData);
		dbg_msg("network_in", "    Unpacker.ErrorMsg() = %s", Unpacker.ErrorMsg());
		return;
	}

	if(NumParts < 1 || NumParts > CSnapshot::MAX_PARTS || Part < 0 || Part >= NumParts)
	{
		dbg_msg("network_in", "  failed to unpack snapshot:");
		dbg_msg("network_in", "    NumParts=%d (has to be in range 1 - CSnapshot::MAX_PARTS (%d))", NumParts, CSnapshot::MAX_PARTS);
		dbg_msg("network_in", "    Part=%d (has to be in range 0 - NumParts (%d))", Part, NumParts);
		return;
	}

	if(PartSize < 0 || PartSize > MAX_SNAPSHOT_PACKSIZE)
	{
		dbg_msg("network_in", "  failed to unpack snapshot:");
		dbg_msg("network_in", "    PartSize=%d (has to be in range 0 - MAX_SNAPSHOT_PACKSIZE (%d))", PartSize, MAX_SNAPSHOT_PACKSIZE);
		return;
	}

	// int local_m_CurrentRecvTick = m_CurrentRecvTick;
	unsigned int local_m_SnapshotParts = m_SnapshotParts;
	char local_m_aSnapshotIncomingData[CSnapshot::MAX_SIZE];

	if(GameTick >= m_CurrentRecvTick)
	{
		if(GameTick != m_CurrentRecvTick)
		{
			local_m_SnapshotParts = 0;
			// local_m_CurrentRecvTick = GameTick;
		}

		// TODO: clean this up abit
		if(pData)
		{
			mem_copy((char*)local_m_aSnapshotIncomingData + Part*MAX_SNAPSHOT_PACKSIZE, pData, PartSize);
		}
		else
		{
			// dbg_msg("network_in", "  unpack snapshot got pData=nil! Is this an error?????");

			// TODO: this might be wrong
			// not sure about this one but i think this is meant for snap empty
			// so it should reuse the old data??
			mem_copy(local_m_aSnapshotIncomingData, m_aSnapshotIncomingData, CSnapshot::MAX_SIZE);
		}
		local_m_SnapshotParts |= 1<<Part;

		if(local_m_SnapshotParts == (unsigned)((1<<NumParts)-1))
		{
			static CSnapshot Emptysnap;
			CSnapshot *pDeltaShot = &Emptysnap;
			int PurgeTick;
			int DeltaSize;
			unsigned char aTmpBuffer2[CSnapshot::MAX_SIZE];
			unsigned char aTmpBuffer3[CSnapshot::MAX_SIZE];
			CSnapshot *pTmpBuffer3 = (CSnapshot*)aTmpBuffer3;	// Fix compiler warning for strict-aliasing
			int SnapSize;

			CompleteSize = (NumParts-1) * MAX_SNAPSHOT_PACKSIZE + PartSize;

			// reset snapshoting
			local_m_SnapshotParts = 0;

			// find snapshot that we should use as delta
			Emptysnap.Clear();

			// find delta
			if(DeltaTick >= 0)
			{
				int DeltashotSize = m_SnapshotStorage.Get(DeltaTick, 0, &pDeltaShot, 0);

				dbg_msg("network_in", "  pDeltaShot->DebugDump() this is the old snap we delta against from the storage:");
				// pDeltaShot->DebugDump();
				debug_dump(pDeltaShot);

				if(DeltashotSize < 0)
				{
					// couldn't find the delta snapshots that the server used
					// to compress this snapshot. force the server to resync
					if(pConfig->m_Debug)
					{
						dbg_msg("network_in", "  error, couldn't find the delta snapshot");
					}

					// ack snapshot
					// TODO: combine this with the input message
					// m_AckGameTick = -1; // no need to change this during print
					return;
				}
			}

			// decompress snapshot
			const void *pDeltaData = m_SnapshotDelta.EmptyDelta();
			DeltaSize = sizeof(int)*3;

			if(CompleteSize)
			{
				int IntSize = CVariableInt::Decompress(local_m_aSnapshotIncomingData, CompleteSize, aTmpBuffer2, sizeof(aTmpBuffer2));

				if(IntSize < 0) // failure during decompression, bail
				{
					for(int e = 0; e < 10; e++)
						dbg_msg("network_in", "  %sERROR DECOMPRESS DATA!!!%s", TERM_RED, TERM_RESET);
					dbg_msg("network_in", "  GameTick=%d IntSize=%d failure during decompression, bail", GameTick, IntSize);

					char aHex[2048];
					str_hex(aHex, sizeof(aHex), local_m_aSnapshotIncomingData, CompleteSize);

					dbg_msg(
						"network_in",
						"  t=%d local_m_aSnapshotIncomingData=%s Size=%d",
						GameTick,
						aHex,
						CompleteSize
					);

					return;
				}

				pDeltaData = aTmpBuffer2;
				DeltaSize = IntSize;
			}

			// unpack delta
			// SnapSize = m_SnapshotDelta.UnpackDelta(pDeltaShot, pTmpBuffer3, pDeltaData, DeltaSize);
			SnapSize = CSnapshotDelta_UnpackDelta(
                pDeltaShot,
                pTmpBuffer3,
                pDeltaData,
                DeltaSize,
#ifdef _PROTOCOL_VERSION_7
                m_SnapshotDelta.m_aItemSizes,
#else
                _gs_aItemSizes,
#endif
				Sixup);
			if(SnapSize < 0)
			{
				dbg_msg("network_in", "  delta unpack failed! (%d)", SnapSize);
				return;
			}

			dbg_msg("network_in", "tmp3->crc=%d gametick=%d", pTmpBuffer3->Crc(), GameTick);
			dbg_msg("network_in", "tmp3->numitems=%d gametick=%d", pTmpBuffer3->NumItems(), GameTick);

			if(pConfig->m_DbgSnapCrc)
			{
				dbg_msg("network_in", "computing crc of snapshot after delta unpack (gametick=%d)", GameTick);
				pTmpBuffer3->VerboseCrc();
			}

			if(Msg != NETMSG_SNAPEMPTY && pTmpBuffer3->Crc() != Crc)
			{
				dbg_msg("network_in", "  snapshot crc error #%d - tick=%d wantedcrc=%d gotcrc=%d compressed_size=%d delta_tick=%d",
					m_SnapCrcErrors, GameTick, Crc, pTmpBuffer3->Crc(), CompleteSize, DeltaTick);

				// m_SnapCrcErrors++;
				if(m_SnapCrcErrors > 10)
				{
					// to many errors, send reset
					// m_AckGameTick = -1;
					// SendInput();
					// m_SnapCrcErrors = 0;
					dbg_msg("network_in", "  too many snap crc erros -> reset");
				}
				return;
			}
			else
			{
				// if(m_SnapCrcErrors)
				// 	m_SnapCrcErrors--;
			}



			// purge old snapshots
			PurgeTick = DeltaTick;
			if(m_aSnapshots[CClient::SNAP_PREV] && m_aSnapshots[CClient::SNAP_PREV]->m_Tick < PurgeTick)
				PurgeTick = m_aSnapshots[CClient::SNAP_PREV]->m_Tick;
			if(m_aSnapshots[CClient::SNAP_CURRENT] && m_aSnapshots[CClient::SNAP_CURRENT]->m_Tick < PurgeTick)
				PurgeTick = m_aSnapshots[CClient::SNAP_CURRENT]->m_Tick;
			dbg_msg("network_in", "  PurgeTick=%d", PurgeTick);
			// m_SnapshotStorage.PurgeUntil(PurgeTick);


			// TODO: chiller uncomment this
			/*

			// add new
			m_SnapshotStorage.Add(GameTick, time_get(), SnapSize, pTmpBuffer3, 1);

			// add snapshot to demo
			if(m_DemoRecorder.IsRecording())
			{
				// build up snapshot and add local messages
				m_DemoRecSnapshotBuilder.Init(pTmpBuffer3);
				GameClient()->OnDemoRecSnap();
				SnapSize = m_DemoRecSnapshotBuilder.Finish(pTmpBuffer3);

				// write snapshot
				m_DemoRecorder.RecordSnapshot(GameTick, pTmpBuffer3, SnapSize);
			}

			// apply snapshot, cycle pointers
			m_ReceivedSnapshots++;

			m_CurrentRecvTick = GameTick;

			// we got two snapshots until we see us self as connected
			if(m_ReceivedSnapshots == 2)
			{
				// start at 200ms and work from there
				m_PredictedTime.Init(GameTick * time_freq() / SERVER_TICK_SPEED);
				m_PredictedTime.SetAdjustSpeed(1, 1000.0f);
				m_GameTime.Init((GameTick - 1) * time_freq() / SERVER_TICK_SPEED);
				m_aSnapshots[CClient::SNAP_PREV] = m_SnapshotStorage.m_pFirst;
				m_aSnapshots[CClient::SNAP_CURRENT] = m_SnapshotStorage.m_pLast;
				SetState(IClient::STATE_ONLINE);
			}
			*/

			// adjust game time
			if(m_ReceivedSnapshots > 2)
			{
				// int64 Now = m_GameTime.Get(time_get());
                int64 Now = 0; // TODO
				int64 TickStart = GameTick * time_freq() / SERVER_TICK_SPEED;
				int64 TimeLeft = (TickStart-Now)*1000 / time_freq();
				// m_GameTime.Update(&m_GametimeMarginGraph, (GameTick - 1) * time_freq() / SERVER_TICK_SPEED, TimeLeft, 0);
				dbg_msg("network_in", "  TickStart=%lld TimeLeft=%lld", TickStart, TimeLeft);
			}

			// ack snapshot
			// m_AckGameTick = GameTick;
		}
	}
}
