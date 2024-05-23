/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/math.h>
#include <base/system.h>

#include <engine/server.h>

#include <engine/shared/compression.h>
#include <engine/shared/network.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>
#include <engine/shared/snapshot.h>

#include "server.h"
#include "game/collision.h"

CSnapIDPool::CSnapIDPool()
{
	Reset();
}

void CSnapIDPool::Reset()
{
	for(int i = 0; i < MAX_IDS; i++)
	{
		m_aIDs[i].m_Next = i+1;
		m_aIDs[i].m_State = 0;
	}

	m_aIDs[MAX_IDS-1].m_Next = -1;
	m_FirstFree = 0;
	m_FirstTimed = -1;
	m_LastTimed = -1;
	m_Usage = 0;
	m_InUsage = 0;
}


void CSnapIDPool::RemoveFirstTimeout()
{
	int NextTimed = m_aIDs[m_FirstTimed].m_Next;

	// add it to the free list
	m_aIDs[m_FirstTimed].m_Next = m_FirstFree;
	m_aIDs[m_FirstTimed].m_State = 0;
	m_FirstFree = m_FirstTimed;

	// remove it from the timed list
	m_FirstTimed = NextTimed;
	if(m_FirstTimed == -1)
		m_LastTimed = -1;

	m_Usage--;
}

int CSnapIDPool::NewID()
{
	int64 Now = time_get();

	// process timed ids
	while(m_FirstTimed != -1 && m_aIDs[m_FirstTimed].m_Timeout < Now)
		RemoveFirstTimeout();

	int ID = m_FirstFree;
	dbg_assert(ID != -1, "id error");
	if(ID == -1)
		return ID;
	m_FirstFree = m_aIDs[m_FirstFree].m_Next;
	m_aIDs[ID].m_State = 1;
	m_Usage++;
	m_InUsage++;
	return ID;
}

void CSnapIDPool::TimeoutIDs()
{
	// process timed ids
	while(m_FirstTimed != -1)
		RemoveFirstTimeout();
}

void CSnapIDPool::FreeID(int ID)
{
	if(ID < 0)
		return;
	dbg_assert(m_aIDs[ID].m_State == 1, "id is not allocated");

	m_InUsage--;
	m_aIDs[ID].m_State = 2;
	m_aIDs[ID].m_Timeout = time_get()+time_freq()*5;
	m_aIDs[ID].m_Next = -1;

	if(m_LastTimed != -1)
	{
		m_aIDs[m_LastTimed].m_Next = ID;
		m_LastTimed = ID;
	}
	else
	{
		m_FirstTimed = ID;
		m_LastTimed = ID;
	}
}

void CServer::CClient::Reset()
{
	// reset input
	for(int i = 0; i < 200; i++)
		m_aInputs[i].m_GameTick = -1;
	m_CurrentInput = 0;
	mem_zero(&m_LatestInput, sizeof(m_LatestInput));

	m_Snapshots.PurgeAll();
	m_LastAckedSnapshot = -1;
	m_LastInputTick = -1;
	m_SnapRate = CClient::SNAPRATE_INIT;
	m_Score = 0;
	m_MapChunk = 0;
}

CServer::CServer()
{
	m_TickSpeed = SERVER_TICK_SPEED;

	m_pGameServer = 0;

	m_CurrentGameTick = 0;
	m_RunServer = true;

	m_pCurrentMapData = 0;
	m_CurrentMapSize = 0;

	m_NumMapEntries = 0;
	m_pFirstMapEntry = 0;
	m_pLastMapEntry = 0;

	m_MapReload = false;

	m_RconClientID = IServer::RCON_CID_SERV;
	m_RconAuthLevel = AUTHED_ADMIN;

	Init();
}

int64 CServer::TickStartTime(int Tick)
{
	return m_GameStartTime + (time_freq()*Tick)/SERVER_TICK_SPEED;
}

int CServer::Init()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		m_aClients[i].m_State = CClient::STATE_EMPTY;
		m_aClients[i].m_aName[0] = 0;
		m_aClients[i].m_aClan[0] = 0;
		m_aClients[i].m_Country = -1;
		m_aClients[i].m_Snapshots.Init();
	}

	m_CurrentGameTick = 0;

	return 0;
}

void CServer::SetRconCID(int ClientID)
{
	m_RconClientID = ClientID;
}

int CServer::GetClientInfo(int ClientID, CClientInfo *pInfo) const
{
	dbg_assert(ClientID >= 0 && ClientID < MAX_CLIENTS, "client_id is not valid");
	dbg_assert(pInfo != 0, "info can not be null");

	if(m_aClients[ClientID].m_State == CClient::STATE_INGAME)
	{
		pInfo->m_pName = m_aClients[ClientID].m_aName;
		pInfo->m_Latency = m_aClients[ClientID].m_Latency;
		return 1;
	}
	return 0;
}

void CServer::GetClientAddr(int ClientID, char *pAddrStr, int Size) const
{
	if(ClientID >= 0 && ClientID < MAX_CLIENTS && m_aClients[ClientID].m_State == CClient::STATE_INGAME)
		net_addr_str(m_NetServer.ClientAddr(ClientID), pAddrStr, Size, false);
}

int CServer::GetClientVersion(int ClientID) const
{
	if(ClientID >= 0 && ClientID < MAX_CLIENTS && m_aClients[ClientID].m_State == CClient::STATE_INGAME)
		return m_aClients[ClientID].m_Version;
	return 0;
}

const char *CServer::ClientName(int ClientID) const
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State == CServer::CClient::STATE_EMPTY)
		return "(invalid)";
	if(m_aClients[ClientID].m_State == CServer::CClient::STATE_INGAME)
		return m_aClients[ClientID].m_aName;
	else
		return "(connecting)";

}

const char *CServer::ClientClan(int ClientID) const
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State == CServer::CClient::STATE_EMPTY)
		return "";
	if(m_aClients[ClientID].m_State == CServer::CClient::STATE_INGAME)
		return m_aClients[ClientID].m_aClan;
	else
		return "";
}

int CServer::ClientCountry(int ClientID) const
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State == CServer::CClient::STATE_EMPTY)
		return -1;
	if(m_aClients[ClientID].m_State == CServer::CClient::STATE_INGAME)
		return m_aClients[ClientID].m_Country;
	else
		return -1;
}

bool CServer::ClientIngame(int ClientID) const
{
	return ClientID >= 0 && ClientID < MAX_CLIENTS && m_aClients[ClientID].m_State == CServer::CClient::STATE_INGAME;
}

int CServer::SendMsg(CMsgPacker *pMsg, int Flags, int ClientID)
{
	CNetChunk Packet;
	if(!pMsg)
		return -1;

	// drop invalid packet
	if(ClientID != -1 && (ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State == CClient::STATE_EMPTY || m_aClients[ClientID].m_Quitting))
		return 0;

	mem_zero(&Packet, sizeof(CNetChunk));
	Packet.m_ClientID = ClientID;
	Packet.m_pData = pMsg->Data();
	Packet.m_DataSize = pMsg->Size();

	if(Flags&MSGFLAG_VITAL)
		Packet.m_Flags |= NETSENDFLAG_VITAL;
	if(Flags&MSGFLAG_FLUSH)
		Packet.m_Flags |= NETSENDFLAG_FLUSH;

	// write message to demo recorder
	if(!(Flags&MSGFLAG_NORECORD))
	{
		// dbg_msg("send", "record");
	}

	if(!(Flags&MSGFLAG_NOSEND))
	{
		if(ClientID == -1)
		{
			// broadcast
			int i;
			for(i = 0; i < MAX_CLIENTS; i++)
				if(m_aClients[i].m_State == CClient::STATE_INGAME && !m_aClients[i].m_Quitting)
				{
					Packet.m_ClientID = i;
					m_NetServer.Send(&Packet);
				}
		}
		else
			m_NetServer.Send(&Packet);
	}
	return 0;
}

void CServer::DoSnapshot()
{
	// create snapshots for all clients
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// client must be ingame to receive snapshots
		if(m_aClients[i].m_State != CClient::STATE_INGAME)
			continue;

		// this client is trying to recover, don't spam snapshots
		if(m_aClients[i].m_SnapRate == CClient::SNAPRATE_RECOVER && (Tick() % SERVER_TICK_SPEED) != 0)
			continue;

		// this client is trying to recover, don't spam snapshots
		if(m_aClients[i].m_SnapRate == CClient::SNAPRATE_INIT && (Tick()%10) != 0)
			continue;

		{
			char aData[CSnapshot::MAX_SIZE];
			CSnapshot *pData = (CSnapshot*)aData;	// Fix compiler warning for strict-aliasing
			char aDeltaData[CSnapshot::MAX_SIZE];
			char aCompData[CSnapshot::MAX_SIZE];
			int SnapshotSize;
			int Crc;
			static CSnapshot EmptySnap;
			CSnapshot *pDeltashot = &EmptySnap;
			int DeltashotSize;
			int DeltaTick = -1;
			int DeltaSize;

			m_SnapshotBuilder.Init();

			GameServer()->OnSnap(i);

			// finish snapshot
			SnapshotSize = m_SnapshotBuilder.Finish(pData);
			Crc = pData->Crc();

			// remove old snapshos
			// keep 3 seconds worth of snapshots
			m_aClients[i].m_Snapshots.PurgeUntil(m_CurrentGameTick-SERVER_TICK_SPEED*3);

			// save it the snapshot
			m_aClients[i].m_Snapshots.Add(m_CurrentGameTick, time_get(), SnapshotSize, pData, 0);

			// find snapshot that we can perform delta against
			EmptySnap.Clear();

			{
				DeltashotSize = m_aClients[i].m_Snapshots.Get(m_aClients[i].m_LastAckedSnapshot, 0, &pDeltashot, 0);
				if(DeltashotSize >= 0)
					DeltaTick = m_aClients[i].m_LastAckedSnapshot;
				else
				{
					// no acked package found, force client to recover rate
					if(m_aClients[i].m_SnapRate == CClient::SNAPRATE_FULL)
						m_aClients[i].m_SnapRate = CClient::SNAPRATE_RECOVER;
				}
			}

			// create delta
			DeltaSize = m_SnapshotDelta.CreateDelta(pDeltashot, pData, aDeltaData);

			if(DeltaSize > 0)
			{
				// compress it
				int SnapshotSize;
				const int MaxSize = MAX_SNAPSHOT_PACKSIZE;
				int NumPackets;

				SnapshotSize = CVariableInt::Compress(aDeltaData, DeltaSize, aCompData, sizeof(aCompData));
				NumPackets = (SnapshotSize+MaxSize-1)/MaxSize;

				for(int n = 0, Left = SnapshotSize; Left > 0; n++)
				{
					int Chunk = Left < MaxSize ? Left : MaxSize;
					Left -= Chunk;

					if(NumPackets == 1)
					{
						CMsgPacker Msg(NETMSG_SNAPSINGLE, true);
						Msg.AddInt(m_CurrentGameTick);
						Msg.AddInt(m_CurrentGameTick-DeltaTick);
						Msg.AddInt(Crc);
						Msg.AddInt(Chunk);
						Msg.AddRaw(&aCompData[n*MaxSize], Chunk);
						SendMsg(&Msg, MSGFLAG_FLUSH, i);
					}
					else
					{
						CMsgPacker Msg(NETMSG_SNAP, true);
						Msg.AddInt(m_CurrentGameTick);
						Msg.AddInt(m_CurrentGameTick-DeltaTick);
						Msg.AddInt(NumPackets);
						Msg.AddInt(n);
						Msg.AddInt(Crc);
						Msg.AddInt(Chunk);
						Msg.AddRaw(&aCompData[n*MaxSize], Chunk);
						SendMsg(&Msg, MSGFLAG_FLUSH, i);
					}
				}
			}
			else
			{
				CMsgPacker Msg(NETMSG_SNAPEMPTY, true);
				Msg.AddInt(m_CurrentGameTick);
				Msg.AddInt(m_CurrentGameTick-DeltaTick);
				SendMsg(&Msg, MSGFLAG_FLUSH, i);

				if(DeltaSize < 0)
				{
					dbg_msg("server", "delta pack failed! (%d)", DeltaSize);
				}
			}
		}
	}
}


int CServer::NewClientCallback(int ClientID, void *pUser)
{
	CServer *pThis = (CServer *)pUser;

	pThis->m_aClients[ClientID].m_State = CClient::STATE_AUTH;
	pThis->m_aClients[ClientID].m_aName[0] = 0;
	pThis->m_aClients[ClientID].m_aClan[0] = 0;
	pThis->m_aClients[ClientID].m_Country = -1;
	pThis->m_aClients[ClientID].m_Authed = AUTHED_NO;
	pThis->m_aClients[ClientID].m_AuthTries = 0;
	pThis->m_aClients[ClientID].m_pMapListEntryToSend = 0;
	pThis->m_aClients[ClientID].m_NoRconNote = false;
	pThis->m_aClients[ClientID].m_Quitting = false;
	pThis->m_aClients[ClientID].m_Latency = 0;
	pThis->m_aClients[ClientID].Reset();

	return 0;
}

void CServer::SendMap(int ClientID)
{
	static const unsigned char DM1_SHA256[] = {0x49, 0x1a, 0xf1, 0x7a, 0x51, 0x02, 0x14, 0x50, 0x62, 0x70, 0x90, 0x4f, 0x14, 0x7a, 0x4c, 0x30, 0xae, 0x0a, 0x85, 0xb9, 0x1b, 0xb8, 0x54, 0x39, 0x5b, 0xef, 0x8c, 0x39, 0x7f, 0xc0, 0x78, 0xc3};
	unsigned int CurrentMapCrc = 1683261464; // should print "crc is 64548818"

	CMsgPacker Msg(NETMSG_MAP_CHANGE, true);
	Msg.AddString(GetMapName(), 0);
	Msg.AddInt(CurrentMapCrc);
	Msg.AddInt(m_CurrentMapSize);
	Msg.AddInt(m_MapChunksPerRequest);
	Msg.AddInt(MAP_CHUNK_SIZE);
	Msg.AddRaw(&DM1_SHA256, sizeof(DM1_SHA256));

	SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
}

void CServer::SendConnectionReady(int ClientID)
{
	CMsgPacker Msg(NETMSG_CON_READY, true);
	SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
}

void CServer::SendRconLine(int ClientID, const char *pLine)
{
	CMsgPacker Msg(NETMSG_RCON_LINE, true);
	Msg.AddString(pLine, 512);
	SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CServer::SendRconLineAuthed(const char *pLine, void *pUser, bool Highlighted)
{
	static bool s_ReentryGuard = false;
	if(s_ReentryGuard)
		return;
	s_ReentryGuard = true;

	CServer *pThis = (CServer *)pUser;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(pThis->m_aClients[i].m_State != CClient::STATE_EMPTY && pThis->m_aClients[i].m_Authed >= pThis->m_RconAuthLevel)
			pThis->SendRconLine(i, pLine);
	}

	s_ReentryGuard = false;
}

// void CServer::SendRconCmdAdd(const IConsole::CCommandInfo *pCommandInfo, int ClientID)
// {
// 	CMsgPacker Msg(NETMSG_RCON_CMD_ADD, true);
// 	Msg.AddString(pCommandInfo->m_pName, IConsole::TEMPCMD_NAME_LENGTH);
// 	Msg.AddString(pCommandInfo->m_pHelp, IConsole::TEMPCMD_HELP_LENGTH);
// 	Msg.AddString(pCommandInfo->m_pParams, IConsole::TEMPCMD_PARAMS_LENGTH);
// 	SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
// }

// void CServer::SendRconCmdRem(const IConsole::CCommandInfo *pCommandInfo, int ClientID)
// {
// 	CMsgPacker Msg(NETMSG_RCON_CMD_REM, true);
// 	Msg.AddString(pCommandInfo->m_pName, 256);
// 	SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
// }

void CServer::SendMapListEntryAdd(const CMapListEntry *pMapListEntry, int ClientID)
{
	CMsgPacker Msg(NETMSG_MAPLIST_ENTRY_ADD, true);
	Msg.AddString(pMapListEntry->m_aName, 256);
	SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CServer::SendMapListEntryRem(const CMapListEntry *pMapListEntry, int ClientID)
{
	CMsgPacker Msg(NETMSG_MAPLIST_ENTRY_REM, true);
	Msg.AddString(pMapListEntry->m_aName, 256);
	SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}


void CServer::UpdateClientMapListEntries()
{
	for(int ClientID = Tick() % MAX_RCONCMD_RATIO; ClientID < MAX_CLIENTS; ClientID += MAX_RCONCMD_RATIO)
	{
		if(m_aClients[ClientID].m_State != CClient::STATE_EMPTY && m_aClients[ClientID].m_Authed)
		{
			for(int i = 0; i < MAX_MAPLISTENTRY_SEND && m_aClients[ClientID].m_pMapListEntryToSend; ++i)
			{
				SendMapListEntryAdd(m_aClients[ClientID].m_pMapListEntryToSend, ClientID);
				m_aClients[ClientID].m_pMapListEntryToSend = m_aClients[ClientID].m_pMapListEntryToSend->m_pNext;
			}
		}
	}
}

void CServer::ProcessClientPacket(CNetChunk *pPacket)
{
	int ClientID = pPacket->m_ClientID;
	CUnpacker Unpacker;
	Unpacker.Reset(pPacket->m_pData, pPacket->m_DataSize);

	// unpack msgid and system flag
	int Msg = Unpacker.GetInt();
	int Sys = Msg&1;
	Msg >>= 1;

	if(Unpacker.Error())
		return;

	if(1)
	{
		const char *pMsg = "unkown";
		if(Msg == NETMSG_NULL) { pMsg = "NULL"; }
		else if(Msg == NETMSG_INFO) { pMsg = "INFO"; }
		else if(Msg == NETMSG_MAP_CHANGE) { pMsg = "MAP_CHANGE"; }
		else if(Msg == NETMSG_MAP_DATA) { pMsg = "MAP_DATA"; }
		else if(Msg == NETMSG_SERVERINFO) { pMsg = "SERVERINFO"; }
		else if(Msg == NETMSG_CON_READY) { pMsg = "CON_READY"; }
		else if(Msg == NETMSG_SNAP) { pMsg = "SNAP"; }
		else if(Msg == NETMSG_SNAPEMPTY) { pMsg = "SNAPEMPTY"; }
		else if(Msg == NETMSG_SNAPSINGLE) { pMsg = "SNAPSINGLE"; }
		else if(Msg == NETMSG_SNAPSMALL) { pMsg = "SNAPSMALL"; }
		else if(Msg == NETMSG_INPUTTIMING) { pMsg = "INPUTTIMING"; }
		else if(Msg == NETMSG_RCON_AUTH_ON) { pMsg = "RCON_AUTH_ON"; }
		else if(Msg == NETMSG_RCON_AUTH_OFF) { pMsg = "RCON_AUTH_OFF"; }
		else if(Msg == NETMSG_RCON_LINE) { pMsg = "RCON_LINE"; }
		else if(Msg == NETMSG_RCON_CMD_ADD) { pMsg = "RCON_CMD_ADD"; }
		else if(Msg == NETMSG_RCON_CMD_REM) { pMsg = "RCON_CMD_REM"; }
		else if(Msg == NETMSG_AUTH_CHALLANGE) { pMsg = "AUTH_CHALLANGE"; }
		else if(Msg == NETMSG_AUTH_RESULT) { pMsg = "AUTH_RESULT"; }
		else if(Msg == NETMSG_READY) { pMsg = "READY"; }
		else if(Msg == NETMSG_ENTERGAME) { pMsg = "ENTERGAME"; }
		else if(Msg == NETMSG_INPUT) { pMsg = "INPUT"; }
		else if(Msg == NETMSG_RCON_CMD) { pMsg = "RCON_CMD"; }
		else if(Msg == NETMSG_RCON_AUTH) { pMsg = "RCON_AUTH"; }
		else if(Msg == NETMSG_REQUEST_MAP_DATA) { pMsg = "REQUEST_MAP_DATA"; }
		else if(Msg == NETMSG_AUTH_START) { pMsg = "AUTH_START"; }
		else if(Msg == NETMSG_AUTH_RESPONSE) { pMsg = "AUTH_RESPONSE"; }
		else if(Msg == NETMSG_PING) { pMsg = "PING"; }
		else if(Msg == NETMSG_PING_REPLY) { pMsg = "PING_REPLY"; }
		else if(Msg == NETMSG_ERROR) { pMsg = "ERROR"; }
		else if(Msg == NETMSG_MAPLIST_ENTRY_ADD) { pMsg = "MAPLIST_ENTRY_ADD"; }
		else if(Msg == NETMSG_MAPLIST_ENTRY_REM) { pMsg = "MAPLIST_ENTRY_REM"; }
		dbg_msg("network_in", "  client packet datasize=%d sys=%d msg=%d (%s)", pPacket->m_DataSize, Sys, Msg, pMsg);
	}


	if(Sys)
	{
		// system message
		if(Msg == NETMSG_INFO)
		{
			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && m_aClients[ClientID].m_State == CClient::STATE_AUTH)
			{
				const char *pVersion = Unpacker.GetString(CUnpacker::SANITIZE_CC);
				if(str_comp(pVersion, GameServer()->NetVersion()) != 0)
				{
					// wrong version
					char aReason[256];
					str_format(aReason, sizeof(aReason), "Wrong version. Server is running '%s' and client '%s'", GameServer()->NetVersion(), pVersion);
					dbg_msg("server", "%s", aReason);
					return;
				}

				const char *pPassword = Unpacker.GetString(CUnpacker::SANITIZE_CC);
				dbg_msg("server", "client sent password=%s", pPassword);
				// wrong password
				// m_NetServer.Drop(ClientID, "Wrong password");
				// return;

				m_aClients[ClientID].m_Version = Unpacker.GetInt();

				m_aClients[ClientID].m_State = CClient::STATE_CONNECTING;
				SendMap(ClientID);
			}
		}
		else if(Msg == NETMSG_REQUEST_MAP_DATA)
		{
			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && (m_aClients[ClientID].m_State == CClient::STATE_CONNECTING || m_aClients[ClientID].m_State == CClient::STATE_CONNECTING_AS_SPEC))
			{
				int ChunkSize = MAP_CHUNK_SIZE;

				// send map chunks
				for(int i = 0; i < m_MapChunksPerRequest && m_aClients[ClientID].m_MapChunk >= 0; ++i)
				{
					int Chunk = m_aClients[ClientID].m_MapChunk;
					int Offset = Chunk * ChunkSize;

					// check for last part
					if(Offset+ChunkSize >= m_CurrentMapSize)
					{
						ChunkSize = m_CurrentMapSize-Offset;
						m_aClients[ClientID].m_MapChunk = -1;
					}
					else
						m_aClients[ClientID].m_MapChunk++;

					CMsgPacker Msg(NETMSG_MAP_DATA, true);
					Msg.AddRaw(&m_pCurrentMapData[Offset], ChunkSize);
					SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);

					char aBuf[64];
					str_format(aBuf, sizeof(aBuf), "sending chunk %d with size %d", Chunk, ChunkSize);
					dbg_msg("server", "%s", aBuf);
				}
			}
		}
		else if(Msg == NETMSG_READY)
		{
			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && (m_aClients[ClientID].m_State == CClient::STATE_CONNECTING || m_aClients[ClientID].m_State == CClient::STATE_CONNECTING_AS_SPEC))
			{
				char aAddrStr[NETADDR_MAXSTRSIZE];
				net_addr_str(m_NetServer.ClientAddr(ClientID), aAddrStr, sizeof(aAddrStr), true);

				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "player is ready. ClientID=%d addr=%s", ClientID, aAddrStr);
				dbg_msg("server", "%s", aBuf);

				bool ConnectAsSpec = m_aClients[ClientID].m_State == CClient::STATE_CONNECTING_AS_SPEC;
				m_aClients[ClientID].m_State = CClient::STATE_READY;
				GameServer()->OnClientConnected(ClientID, ConnectAsSpec);
				SendConnectionReady(ClientID);
			}
		}
		else if(Msg == NETMSG_ENTERGAME)
		{
			SendServerInfo(ClientID);
			GameServer()->OnClientEnter(ClientID);
		}
		else if(Msg == NETMSG_INPUT)
		{
			int64 Now = time_get();

			m_aClients[ClientID].m_LastAckedSnapshot = Unpacker.GetInt();
			int IntendedTick = Unpacker.GetInt();
			int Size = Unpacker.GetInt();

			// check for errors
			if(Unpacker.Error() || Size/4 > MAX_INPUT_SIZE)
				return;

			// add message to report the input timing
			// skip packets that are old
			if(IntendedTick > m_aClients[ClientID].m_LastInputTick)
			{
				int TimeLeft = ((TickStartTime(IntendedTick)-Now)*1000) / time_freq();

				CMsgPacker Msg(NETMSG_INPUTTIMING, true);
				Msg.AddInt(IntendedTick);
				Msg.AddInt(TimeLeft);
				SendMsg(&Msg, 0, ClientID);
			}

			for(int i = 0; i < Size/4; i++)
			{
				int InputData = Unpacker.GetInt();
				dbg_msg("server", "input data = %d", InputData);
			}

			int PingCorrection = clamp(Unpacker.GetInt(), 0, 50);
			dbg_msg("server", "ping correction = %d", PingCorrection);
		}
		else if(Msg == NETMSG_RCON_CMD)
		{
			const char *pCmd = Unpacker.GetString();

			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Unpacker.Error() == 0 && m_aClients[ClientID].m_Authed)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "ClientID=%d rcon='%s'", ClientID, pCmd);
				dbg_msg("server", "%s", aBuf);
				m_RconClientID = ClientID;
				m_RconAuthLevel = m_aClients[ClientID].m_Authed;
				m_RconClientID = IServer::RCON_CID_SERV;
				m_RconAuthLevel = AUTHED_ADMIN;
			}
		}
		else if(Msg == NETMSG_RCON_AUTH)
		{
			const char *pPw = Unpacker.GetString(CUnpacker::SANITIZE_CC);
			dbg_msg("server", "client send rcon password=%s", pPw);

			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Unpacker.Error() == 0)
			{
				SendRconLine(ClientID, "Wrong password.");
			}
		}
		else if(Msg == NETMSG_PING)
		{
			CMsgPacker Msg(NETMSG_PING_REPLY, true);
			SendMsg(&Msg, 0, ClientID);
		}
		else
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "strange message ClientID=%d msg=%d data_size=%d", ClientID, Msg, pPacket->m_DataSize);
			dbg_msg("server", "%s", aBuf);
			str_hex(aBuf, sizeof(aBuf), pPacket->m_pData, minimum(pPacket->m_DataSize, 32));
			dbg_msg("server", "%s", aBuf);
		}
	}
	else
	{
		// game message
		if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && m_aClients[ClientID].m_State >= CClient::STATE_READY)
			GameServer()->OnMessage(Msg, &Unpacker, ClientID);
	}
}

void CServer::GenerateServerInfo(CPacker *pPacker, int Token)
{
	// count the players
	int PlayerCount = 0, ClientCount = 0;

	if(Token != -1)
	{
		static const unsigned char SERVERBROWSE_INFO[] = {255, 255, 255, 255, 'i', 'n', 'f', '3'};
		pPacker->Reset();
		pPacker->AddRaw(SERVERBROWSE_INFO, sizeof(SERVERBROWSE_INFO));
		pPacker->AddInt(Token);
	}

	pPacker->AddString(GameServer()->Version(), 32);
	pPacker->AddString(GetServerName(), 64);
	pPacker->AddString("127.0.0.1", 128); // hostname
	pPacker->AddString(GetMapName(), 32);

	// gametype
	pPacker->AddString(GameServer()->GameType(), 16);

	// flags
	int Flags = 0;
	Flags |= SERVERINFO_FLAG_PASSWORD;
	Flags |= SERVERINFO_FLAG_TIMESCORE;
	pPacker->AddInt(Flags);

	pPacker->AddInt(0);	// server skill level
	pPacker->AddInt(PlayerCount); // num players
	pPacker->AddInt(10); // max players
	pPacker->AddInt(ClientCount); // num clients
	pPacker->AddInt(64); // max clients

	if(Token != -1)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_aClients[i].m_State != CClient::STATE_EMPTY)
			{
				pPacker->AddString(ClientName(i), 0); // client name
				pPacker->AddString(ClientClan(i), 0); // client clan
				pPacker->AddInt(m_aClients[i].m_Country); // client country
				pPacker->AddInt(m_aClients[i].m_Score); // client score
				pPacker->AddInt(0); // flag spectator=1, bot=2 (player=0)
			}
		}
	}
}

void CServer::SendServerInfo(int ClientID)
{
	CMsgPacker Msg(NETMSG_SERVERINFO, true);
	GenerateServerInfo(&Msg, -1);
	if(ClientID == -1)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_aClients[i].m_State != CClient::STATE_EMPTY)
				SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH, i);
		}
	}
	else if(ClientID >= 0 && ClientID < MAX_CLIENTS && m_aClients[ClientID].m_State != CClient::STATE_EMPTY)
		SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
}


void CServer::PumpNetwork()
{
	CNetChunk Packet;
	TOKEN ResponseToken;

	static const unsigned char SERVERBROWSE_GETINFO[] = {255, 255, 255, 255, 'g', 'i', 'e', '3'};


	// process packets
	while(m_NetServer.Recv(&Packet, &ResponseToken))
	{
		if(Packet.m_Flags&NETSENDFLAG_CONNLESS)
		{
			if(Packet.m_DataSize >= int(sizeof(SERVERBROWSE_GETINFO)) &&
				mem_comp(Packet.m_pData, SERVERBROWSE_GETINFO, sizeof(SERVERBROWSE_GETINFO)) == 0)
			{
				CUnpacker Unpacker;
				Unpacker.Reset((unsigned char*)Packet.m_pData+sizeof(SERVERBROWSE_GETINFO), Packet.m_DataSize-sizeof(SERVERBROWSE_GETINFO));
				int SrvBrwsToken = Unpacker.GetInt();
				if(Unpacker.Error())
					continue;

				CPacker Packer;
				CNetChunk Response;

				GenerateServerInfo(&Packer, SrvBrwsToken);

				Response.m_ClientID = -1;
				Response.m_Address = Packet.m_Address;
				Response.m_Flags = NETSENDFLAG_CONNLESS;
				Response.m_pData = Packer.Data();
				Response.m_DataSize = Packer.Size();
				m_NetServer.Send(&Response, ResponseToken);
			}
		}
		else
			ProcessClientPacket(&Packet);
	}
}

const char *CServer::GetMapName()
{
	return "dm1";
}

const char *CServer::GetServerName()
{
	return "unnamed server";
}

void CServer::Init(IGameServer *pGameServer)
{
	m_pGameServer = pGameServer;
}

#include <chrono>
#include <thread>
#include <dlfcn.h>

#include <bots/bot.h>
#include <cstdio>

void *LoadTick(FBotTick *pfnBotTick)
{
	*pfnBotTick = nullptr;

	dlerror(); // clear old error
	void *pHandle = dlopen("./libtwbl_bottick.so", RTLD_NOW | RTLD_GLOBAL);
	const char *pError = dlerror();
	if(!pHandle || pError)
	{
		fprintf(stderr, "dlopen failed: %s\n", pError);
		if(pHandle)
			dlclose(pHandle);
		return nullptr;
	}

	*pfnBotTick = (FBotTick)dlsym(pHandle, "BotTick");
	pError = dlerror();
	if(!*pfnBotTick || pError)
	{
		fprintf(stderr, "dlsym failed: %s\n", pError);
		if(pHandle)
			dlclose(pHandle);
		return nullptr;
	}
	return pHandle;
}

void Sleep(int Miliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(Miliseconds));
}

int CServer::Run(bool shutdown)
{
	dbg_msg("server", "hello world");

	CCollision Col;
	Col.Init();

	while(true)
	{
		FBotTick pfnBotTick;
		void *pHandle = LoadTick(&pfnBotTick);
		if(pHandle)
		{
			pfnBotTick(&Col);
			dlclose(pHandle);
		}
		else
		{
			BotTick(&Col);
		}

		Sleep(20);
	}

	return 0;
}

int CServer::SnapNewID()
{
	return m_IDPool.NewID();
}

void CServer::SnapFreeID(int ID)
{
	m_IDPool.FreeID(ID);
}


void *CServer::SnapNewItem(int Type, int ID, int Size)
{
	dbg_assert(Type >= 0 && Type <=0xffff, "incorrect type");
	dbg_assert(ID >= 0 && ID <=0xffff, "incorrect id");
	return ID < 0 ? 0 : m_SnapshotBuilder.NewItem(Type, ID, Size);
}

void CServer::SnapSetStaticsize(int ItemType, int Size)
{
	m_SnapshotDelta.SetStaticsize(ItemType, Size);
}

static CServer *CreateServer() { return new CServer(); }

int main(int argc, const char **argv) // ignore_convention
{
	bool shutdown = false;
	for(int i = 1; i < argc; i++) // ignore_convention
	{
		if(str_comp("shutdown", argv[i]) == 0) // ignore_convention
		{
			shutdown = true;
		}
	}

	CServer *pServer = CreateServer();

	IGameServer *pGameServer = CreateGameServer();
	pServer->Init(pGameServer);
	pGameServer->OnInit(pServer);

	// run the server
	dbg_msg("server", "starting...");
	int Ret = pServer->Run(shutdown);

	// free
	delete pServer;
	delete pGameServer;

	return Ret;
}
