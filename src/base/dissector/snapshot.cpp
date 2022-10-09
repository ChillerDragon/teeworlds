// ChillerDragon 2022 - dissector

#include "byte_printer.h"
#include "dissector.h"

#include <engine/shared/packer.h>
#include <engine/shared/network.h>
#include <base/math.h>
#include <engine/shared/config.h>

#include <new>
#include <algorithm>

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
#include <engine/shared/protocol.h>
#include <limits.h>

#include "compat.h"

#include "snapshot.h"

static int _RangeCheck(const void *pEnd, const void *pPtr, int Size)
{
	if((const char *)pPtr + Size > (const char *)pEnd)
		return -1;
	return 0;
}

void debug_dump(CSnapshot *pSnapShot)
{
	dbg_msg("snapshot", "num_items=%d", pSnapShot->NumItems());
	for(int i = 0; i < pSnapShot->NumItems(); i++)
	{
		const CSnapshotItem *pItem = pSnapShot->GetItem(i);
		int Size = pSnapShot->GetItemSize(i);
		char aType[128];
		netobj_to_str(pItem->Type(), aType, sizeof(aType));
		dbg_msg("snapshot", "\ttype=%d (%s)  id=%d", pItem->Type(), aType, pItem->ID());

		int b = 0;
		if(pItem->Type() == _NETOBJTYPE_PLAYERINFO)
		{
			if(Size != 12)
			{
				dbg_msg("snapshot", "\t\tInvalid Size=%d Expected=12", Size);
				exit(1);
			}
			const CNetObj_PlayerInfo *pChr = ((const CNetObj_PlayerInfo *)pItem->Data());

			dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_PlayerFlags=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_PlayerFlags);b++;
			dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Score=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Score);b++;
			dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_Latency=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_Latency);b++;
		}
		else if(pItem->Type() == _NETOBJTYPE_GAMEDATA)
		{
			if(Size != 12)
			{
				dbg_msg("snapshot", "\t\tInvalid Size=%d Expected=12", Size);
				exit(1);
			}
			const CNetObj_GameData *pChr = ((const CNetObj_GameData *)pItem->Data());

			dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_GameStartTick=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_GameStartTick);b++;
			dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_GameStateFlags=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_GameStateFlags);b++;
			dbg_msg("snapshot", "\t\t%3d %12d\t%08x\t m_GameStateEndTick=%d", b, pItem->Data()[b], pItem->Data()[b], pChr->m_GameStateEndTick);b++;
		}
		else if(pItem->Type() == _NETOBJTYPE_CHARACTER)
		{
			if(Size != 88)
			{
				dbg_msg("snapshot", "\t\tInvalid Size=%d Expected=88", Size);
				exit(1);
			}
			const CNetObj_Character *pChr = ((const CNetObj_Character *)pItem->Data());

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
			for(b = 0; b < Size / 4; b++)
				dbg_msg("snapshot", "\t\t%3d %12d\t%08x", b, pItem->Data()[b], pItem->Data()[b]);
		}
	}
}

void netobj_to_str(int Type, char *pBuf, int Size)
{
  init_compat();
  const char *pType = "unkown";
  if(Type == _NETOBJ_INVALID) pType = "NETOBJ_INVALID";
  if(Type == _NETOBJTYPE_PLAYERINPUT) pType = "NETOBJTYPE_PLAYERINPUT";
  if(Type == _NETOBJTYPE_PROJECTILE) pType = "NETOBJTYPE_PROJECTILE";
  if(Type == _NETOBJTYPE_LASER) pType = "NETOBJTYPE_LASER";
  if(Type == _NETOBJTYPE_PICKUP) pType = "NETOBJTYPE_PICKUP";
  if(Type == _NETOBJTYPE_FLAG) pType = "NETOBJTYPE_FLAG";
  if(Type == _NETOBJTYPE_GAMEDATA) pType = "NETOBJTYPE_GAMEDATA";
  if(Type == _NETOBJTYPE_GAMEDATATEAM) pType = "NETOBJTYPE_GAMEDATATEAM";
  if(Type == _NETOBJTYPE_GAMEDATAFLAG) pType = "NETOBJTYPE_GAMEDATAFLAG";
  if(Type == _NETOBJTYPE_CHARACTERCORE) pType = "NETOBJTYPE_CHARACTERCORE";
  if(Type == _NETOBJTYPE_CHARACTER) pType = "NETOBJTYPE_CHARACTER";
  if(Type == _NETOBJTYPE_PLAYERINFO) pType = "NETOBJTYPE_PLAYERINFO";
  if(Type == _NETOBJTYPE_SPECTATORINFO) pType = "NETOBJTYPE_SPECTATORINFO";
  if(Type == _NETOBJTYPE_DE_CLIENTINFO) pType = "NETOBJTYPE_DE_CLIENTINFO";
  if(Type == _NETOBJTYPE_DE_GAMEINFO) pType = "NETOBJTYPE_DE_GAMEINFO";
  if(Type == _NETOBJTYPE_DE_TUNEPARAMS) pType = "NETOBJTYPE_DE_TUNEPARAMS";
  if(Type == _NETEVENTTYPE_COMMON) pType = "NETEVENTTYPE_COMMON";
  if(Type == _NETEVENTTYPE_EXPLOSION) pType = "NETEVENTTYPE_EXPLOSION";
  if(Type == _NETEVENTTYPE_SPAWN) pType = "NETEVENTTYPE_SPAWN";
  if(Type == _NETEVENTTYPE_HAMMERHIT) pType = "NETEVENTTYPE_HAMMERHIT";
  if(Type == _NETEVENTTYPE_DEATH) pType = "NETEVENTTYPE_DEATH";
  if(Type == _NETEVENTTYPE_SOUNDWORLD) pType = "NETEVENTTYPE_SOUNDWORLD";
  if(Type == _NETEVENTTYPE_DAMAGE) pType = "NETEVENTTYPE_DAMAGE";
  if(Type == _NETOBJTYPE_PLAYERINFORACE) pType = "NETOBJTYPE_PLAYERINFORACE";
  if(Type == _NETOBJTYPE_GAMEDATARACE) pType = "NETOBJTYPE_GAMEDATARACE";

  str_copy(pBuf, pType, Size);
}

int CSnapshotDelta_UnpackDelta(const CSnapshot *pFrom, CSnapshot *pTo, const void *pSrcData, int DataSize, const short *ppItemSizes, bool Sixup)
{
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
	int ID, Type, Key;
	int FromIndex;
	int *pNewData;

	Builder.Init();

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
		return -1;
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
			return -1;

		Type = *pData++;
		if(Type < 0 || Type > CSnapshot::MAX_TYPE)
			return -3;

		ID = *pData++;
		if(ID < 0 || ID > CSnapshot::MAX_ID)
			return -3;

		if(Type < _MAX_NETOBJSIZES && ppItemSizes[Type])
			ItemSize = ppItemSizes[Type];
		else
		{
			if(pData+1 > pEnd)
				return -2;
			if(*pData < 0 || *pData > INT_MAX / 4)
				return -3;
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

		Key = (Type<<16)|(ID&0xffff);

		// create the item if needed
		pNewData = Builder.GetItemData(Key);
		if(!pNewData)
			pNewData = (int *)Builder.NewItem(Key>>16, Key&0xffff, ItemSize);

		//if(range_check(pEnd, pNewData, ItemSize)) return -4;

		FromIndex = pFrom->GetItemIndex(Key);
		if(FromIndex != -1)
		{
			// we got an update so we need to apply the diff
			// UndiffItem(pFrom->GetItem(FromIndex)->Data(), pData, pNewData, ItemSize / 4, &m_aSnapshotDataRate[Type]);
			// m_aSnapshotDataUpdates[Type]++;
		}
		else // no previous, just copy the pData
		{
			mem_copy(pNewData, pData, ItemSize);
			// m_aSnapshotDataRate[Type] += ItemSize * 8;
			// m_aSnapshotDataUpdates[Type]++;
		}

		pData += ItemSize/4;

		dbg_msg("network_in", "    UnpackItem of size=%d", ItemSize);
    char aType[128];
		netobj_to_str(Type, aType, sizeof(aType));
		int PrintItemLen = minimum(20, ItemSize);
		char aCutNote[512];
		char aTypeNote[512];
		char aIdNote[512];
		str_format(aTypeNote, sizeof(aTypeNote), "Type=%d (%s)", Type, aType);
		str_format(aIdNote, sizeof(aIdNote), "ID=%d", ID);
		aCutNote[0] = '\0';
		if(ItemSize != PrintItemLen)
			str_format(aCutNote, sizeof(aCutNote), "[CUT OFF %d BYTES]", ItemSize - PrintItemLen);
		print_hex_row_highlight_two(
			"network_in",
			"    ",
			pItemData, PrintItemLen,
			0, 0,
			aTypeNote,
			1, 1,
			aIdNote,
			aCutNote);
	}

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

	dbg_msg("network_in", "SnapShot Msg=%d (%s) receivedSnaps=%d", Msg, pMsg, m_ReceivedSnapshots);
	dbg_msg("network_in", "  gametick=%d", GameTick);
	dbg_msg("network_in", "  deltatick=%d", DeltaTick);

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
		dbg_msg("network_in", "  NETMSG_SNAPEMPTY Crc=%x PartSize=%d", Crc, PartSize);
	}

	pData = (const char *)Unpacker.GetRaw(PartSize);

	if(Unpacker.Error() || NumParts < 1 || NumParts > CSnapshot::MAX_PARTS || Part < 0 || Part >= NumParts || PartSize < 0 || PartSize > MAX_SNAPSHOT_PACKSIZE)
	{
		dbg_msg("network_in", "  failed to unpack snapshot:");
		dbg_msg("network_in", "    Unpacker.Error() = %d", Unpacker.Error());
		dbg_msg("network_in", "    NumParts=%d (has to be in range 1 - CSnapshot::MAX_PARTS (%d))", NumParts, CSnapshot::MAX_PARTS);
		dbg_msg("network_in", "    Part=%d (has to be in range 0 - NumParts (%d))", Part, NumParts);
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
		mem_copy((char*)local_m_aSnapshotIncomingData + Part*MAX_SNAPSHOT_PACKSIZE, pData, PartSize);
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

				dbg_msg("network_in", "  pDeltaShot->DebugDump():");
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
				int IntSize = CVariableInt::Decompress(m_aSnapshotIncomingData, CompleteSize, aTmpBuffer2, sizeof(aTmpBuffer2));

				if(IntSize < 0) // failure during decompression, bail
				{
					dbg_msg("network_in", "  IntSize=%d failure during decompression, bail", IntSize);
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

			if(Msg != NETMSG_SNAPEMPTY && pTmpBuffer3->Crc() != Crc)
			{
				if(pConfig->m_Debug)
				{
					dbg_msg("network_in", "  snapshot crc error #%d - tick=%d wantedcrc=%d gotcrc=%d compressed_size=%d delta_tick=%d",
						m_SnapCrcErrors, GameTick, Crc, pTmpBuffer3->Crc(), CompleteSize, DeltaTick);
				}

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
