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
#include <engine/masterserver.h>
#include <engine/serverbrowser.h>
#include <engine/sound.h>
#include <engine/storage.h>
#include <engine/textrender.h>

#include <engine/shared/config.h>
#include <engine/shared/compression.h>
#include <engine/shared/datafile.h>
#include <engine/shared/demo.h>
#include <engine/shared/filecollection.h>
#include <engine/shared/mapchecker.h>
#include <engine/shared/network.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>
#include <engine/shared/ringbuffer.h>
#include <engine/shared/snapshot.h>

#include <game/version.h>

#include <mastersrv/mastersrv.h>
#include <versionsrv/versionsrv.h>

#include <engine/contacts.h>
#include <engine/serverbrowser.h>
#include <engine/client/serverbrowser.h>
#include <engine/client/contacts.h>
#include <engine/client.h>
#include <engine/client/client.h>

#include <generated/protocol.h>
#include <engine/shared/protocol.h>
#include <limits.h>

int CSnapshotDelta_UnpackDelta(const CSnapshot *pFrom, CSnapshot *pTo, const void *pSrcData, int DataSize, const short *ppItemSizes)
{
	const int MAX_NETOBJSIZES = 64;
	CSnapshotBuilder Builder;
	const CSnapshotDelta::CData *pDelta = (const CSnapshotDelta::CData *)pSrcData;
	const int *pData = (const int *)pDelta->m_pData;
	const int *pEnd = (const int *)(((const char *)pSrcData + DataSize));

	const CSnapshotItem *pFromItem;
	int Keep, ItemSize;
	const int *pDeleted;
	int ID, Type, Key;
	int FromIndex;
	int *pNewData;

	Builder.Init();

	dbg_msg("network_in", "  CSnapshotDelta_UnpackDelta unpacking %d items:", pDelta->m_NumUpdateItems);
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

		if(Type < MAX_NETOBJSIZES && ppItemSizes[Type])
			ItemSize = ppItemSizes[Type];
		else
		{
			if(pData+1 > pEnd)
				return -2;
			if(*pData < 0 || *pData > INT_MAX / 4)
				return -3;
			ItemSize = (*pData++) * 4;
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
		const char *pType = "unkown";
		if(Type == NETOBJ_INVALID) pType = "NETOBJ_INVALID";
		if(Type == NETOBJTYPE_PLAYERINPUT) pType = "NETOBJTYPE_PLAYERINPUT";
		if(Type == NETOBJTYPE_PROJECTILE) pType = "NETOBJTYPE_PROJECTILE";
		if(Type == NETOBJTYPE_LASER) pType = "NETOBJTYPE_LASER";
		if(Type == NETOBJTYPE_PICKUP) pType = "NETOBJTYPE_PICKUP";
		if(Type == NETOBJTYPE_FLAG) pType = "NETOBJTYPE_FLAG";
		if(Type == NETOBJTYPE_GAMEDATA) pType = "NETOBJTYPE_GAMEDATA";
		if(Type == NETOBJTYPE_GAMEDATATEAM) pType = "NETOBJTYPE_GAMEDATATEAM";
		if(Type == NETOBJTYPE_GAMEDATAFLAG) pType = "NETOBJTYPE_GAMEDATAFLAG";
		if(Type == NETOBJTYPE_CHARACTERCORE) pType = "NETOBJTYPE_CHARACTERCORE";
		if(Type == NETOBJTYPE_CHARACTER) pType = "NETOBJTYPE_CHARACTER";
		if(Type == NETOBJTYPE_PLAYERINFO) pType = "NETOBJTYPE_PLAYERINFO";
		if(Type == NETOBJTYPE_SPECTATORINFO) pType = "NETOBJTYPE_SPECTATORINFO";
		if(Type == NETOBJTYPE_DE_CLIENTINFO) pType = "NETOBJTYPE_DE_CLIENTINFO";
		if(Type == NETOBJTYPE_DE_GAMEINFO) pType = "NETOBJTYPE_DE_GAMEINFO";
		if(Type == NETOBJTYPE_DE_TUNEPARAMS) pType = "NETOBJTYPE_DE_TUNEPARAMS";
		if(Type == NETEVENTTYPE_COMMON) pType = "NETEVENTTYPE_COMMON";
		if(Type == NETEVENTTYPE_EXPLOSION) pType = "NETEVENTTYPE_EXPLOSION";
		if(Type == NETEVENTTYPE_SPAWN) pType = "NETEVENTTYPE_SPAWN";
		if(Type == NETEVENTTYPE_HAMMERHIT) pType = "NETEVENTTYPE_HAMMERHIT";
		if(Type == NETEVENTTYPE_DEATH) pType = "NETEVENTTYPE_DEATH";
		if(Type == NETEVENTTYPE_SOUNDWORLD) pType = "NETEVENTTYPE_SOUNDWORLD";
		if(Type == NETEVENTTYPE_DAMAGE) pType = "NETEVENTTYPE_DAMAGE";
		if(Type == NETOBJTYPE_PLAYERINFORACE) pType = "NETOBJTYPE_PLAYERINFORACE";
		if(Type == NETOBJTYPE_GAMEDATARACE) pType = "NETOBJTYPE_GAMEDATARACE";
		if(Type == NUM_NETOBJTYPES) pType = "NUM_NETOBJTYPES";
		int PrintItemLen = minimum(20, ItemSize);
		char aCutNote[512];
		char aTypeNote[512];
		char aIdNote[512];
		str_format(aTypeNote, sizeof(aTypeNote), "Type=%d (%s)", Type, pType);
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
    const CSnapshotStorage &m_SnapshotStorage,
    int m_SnapCrcErrors,
    // const CSmoothTime &GameTime,
    const char *m_aSnapshotIncomingData,
    CSnapshotStorage::CHolder **m_aSnapshots,
    CClient *pClient)
{
	if(pConfig->m_Debug < 2)
		return;

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
				pDeltaShot->DebugDump();

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
			SnapSize = CSnapshotDelta_UnpackDelta(pDeltaShot, pTmpBuffer3, pDeltaData, DeltaSize, m_SnapshotDelta.m_aItemSizes);
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
