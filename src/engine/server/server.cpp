/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/math.h>
#include <base/system.h>

#include <engine/config.h>
#include <engine/engine.h>
#include <engine/server.h>

#include <engine/shared/compression.h>
#include <engine/shared/config.h>
#include <engine/shared/network.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>
#include <engine/shared/snapshot.h>

#include "register.h"
#include "server.h"

#if defined(CONF_FAMILY_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#include <signal.h>

volatile sig_atomic_t InterruptSignaled = 0;

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

	str_copy(m_aShutdownReason, "Server shutdown", sizeof(m_aShutdownReason));

	m_pCurrentMapData = 0;
	m_CurrentMapSize = 0;

	m_NumMapEntries = 0;
	m_pFirstMapEntry = 0;
	m_pLastMapEntry = 0;
	m_pMapListHeap = 0;

	m_MapReload = false;

	m_RconClientID = IServer::RCON_CID_SERV;
	m_RconAuthLevel = AUTHED_ADMIN;

	m_RconPasswordSet = 0;
	m_GeneratedRconPassword = 0;

	Init();
}


void CServer::SetClientName(int ClientID, const char *pName)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY || !pName)
		return;

	const char *pDefaultName = "(1)";
	pName = str_utf8_skip_whitespaces(pName);
	str_utf8_copy_num(m_aClients[ClientID].m_aName, *pName ? pName : pDefaultName, sizeof(m_aClients[ClientID].m_aName), MAX_NAME_LENGTH);
}

void CServer::SetClientClan(int ClientID, const char *pClan)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY || !pClan)
		return;

	str_utf8_copy_num(m_aClients[ClientID].m_aClan, pClan, sizeof(m_aClients[ClientID].m_aClan), MAX_CLAN_LENGTH);
}

void CServer::SetClientCountry(int ClientID, int Country)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY)
		return;

	m_aClients[ClientID].m_Country = Country;
}

void CServer::SetClientScore(int ClientID, int Score)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State < CClient::STATE_READY)
		return;
	m_aClients[ClientID].m_Score = Score;
}

void CServer::Kick(int ClientID, const char *pReason)
{
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || m_aClients[ClientID].m_State == CClient::STATE_EMPTY)
	{
		dbg_msg("server", "invalid client id to kick");
		return;
	}
	else if(m_RconClientID == ClientID)
	{
		dbg_msg("server", "you can't kick yourself");
 		return;
	}
	else if(m_aClients[ClientID].m_Authed > m_RconAuthLevel)
	{
		dbg_msg("server", "kick command denied");
 		return;
	}

	m_NetServer.Drop(ClientID, pReason);
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

bool CServer::IsAuthed(int ClientID) const
{
	return m_aClients[ClientID].m_Authed;
}

bool CServer::IsBanned(int ClientID)
{
	return false;
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

void CServer::InitRconPasswordIfUnset()
{
	if(m_RconPasswordSet)
	{
		return;
	}

	static const char VALUES[] = "ABCDEFGHKLMNPRSTUVWXYZabcdefghjkmnopqt23456789";
	static const size_t NUM_VALUES = sizeof(VALUES) - 1; // Disregard the '\0'.
	static const size_t PASSWORD_LENGTH = 6;
	dbg_assert(NUM_VALUES * NUM_VALUES >= 2048, "need at least 2048 possibilities for 2-character sequences");
	// With 6 characters, we get a password entropy of log(2048) * 6/2 = 33bit.

	dbg_assert(PASSWORD_LENGTH % 2 == 0, "need an even password length");
	unsigned short aRandom[PASSWORD_LENGTH / 2];
	char aRandomPassword[PASSWORD_LENGTH+1];
	aRandomPassword[PASSWORD_LENGTH] = 0;

	secure_random_fill(aRandom, sizeof(aRandom));
	for(size_t i = 0; i < PASSWORD_LENGTH / 2; i++)
	{
		unsigned short RandomNumber = aRandom[i] % 2048;
		aRandomPassword[2 * i + 0] = VALUES[RandomNumber / NUM_VALUES];
		aRandomPassword[2 * i + 1] = VALUES[RandomNumber % NUM_VALUES];
	}

	str_copy(Config()->m_SvRconPassword, aRandomPassword, sizeof(Config()->m_SvRconPassword));
	m_GeneratedRconPassword = 1;
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
	GameServer()->OnPreSnap();

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

	GameServer()->OnPostSnap();
}


int CServer::NewClientCallback(int ClientID, void *pUser)
{
	CServer *pThis = (CServer *)pUser;

	// Remove non human player on same slot
	if(pThis->GameServer()->IsClientBot(ClientID))
	{
		pThis->GameServer()->OnClientDrop(ClientID, "removing dummy");
	}

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

int CServer::DelClientCallback(int ClientID, const char *pReason, void *pUser)
{
	CServer *pThis = (CServer *)pUser;

	char aAddrStr[NETADDR_MAXSTRSIZE];
	net_addr_str(pThis->m_NetServer.ClientAddr(ClientID), aAddrStr, sizeof(aAddrStr), true);
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "client dropped. cid=%d addr=%s reason='%s'", ClientID, aAddrStr, pReason);
	dbg_msg("server", "%s", aBuf);

	// notify the mod about the drop
	if(pThis->m_aClients[ClientID].m_State >= CClient::STATE_READY)
	{
		pThis->m_aClients[ClientID].m_Quitting = true;
		pThis->GameServer()->OnClientDrop(ClientID, pReason);
	}

	pThis->m_aClients[ClientID].m_State = CClient::STATE_EMPTY;
	pThis->m_aClients[ClientID].m_aName[0] = 0;
	pThis->m_aClients[ClientID].m_aClan[0] = 0;
	pThis->m_aClients[ClientID].m_Country = -1;
	pThis->m_aClients[ClientID].m_Authed = AUTHED_NO;
	pThis->m_aClients[ClientID].m_AuthTries = 0;
	pThis->m_aClients[ClientID].m_pMapListEntryToSend = 0;
	pThis->m_aClients[ClientID].m_NoRconNote = false;
	pThis->m_aClients[ClientID].m_Quitting = false;
	pThis->m_aClients[ClientID].m_Snapshots.PurgeAll();
	return 0;
}

void CServer::SendMap(int ClientID)
{
	CMsgPacker Msg(NETMSG_MAP_CHANGE, true);
	Msg.AddString(GetMapName(), 0);
	Msg.AddInt(m_CurrentMapCrc);
	Msg.AddInt(m_CurrentMapSize);
	Msg.AddInt(m_MapChunksPerRequest);
	Msg.AddInt(MAP_CHUNK_SIZE);
	Msg.AddRaw(&m_CurrentMapSha256, sizeof(m_CurrentMapSha256));
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

	if(m_pConfig->m_Debug > 2)
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
					m_NetServer.Drop(ClientID, aReason);
					return;
				}

				const char *pPassword = Unpacker.GetString(CUnpacker::SANITIZE_CC);
				if(Config()->m_Password[0] != 0 && str_comp(Config()->m_Password, pPassword) != 0)
				{
					// wrong password
					m_NetServer.Drop(ClientID, "Wrong password");
					return;
				}

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

					if(Config()->m_Debug)
					{
						char aBuf[64];
						str_format(aBuf, sizeof(aBuf), "sending chunk %d with size %d", Chunk, ChunkSize);
						dbg_msg("server", "%s", aBuf);
					}
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
			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && m_aClients[ClientID].m_State == CClient::STATE_READY && GameServer()->IsClientReady(ClientID))
			{
				char aAddrStr[NETADDR_MAXSTRSIZE];
				net_addr_str(m_NetServer.ClientAddr(ClientID), aAddrStr, sizeof(aAddrStr), true);

				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "player has entered the game. ClientID=%d addr=%s", ClientID, aAddrStr);
				dbg_msg("server", "%s", aBuf);
				m_aClients[ClientID].m_State = CClient::STATE_INGAME;
				SendServerInfo(ClientID);
				GameServer()->OnClientEnter(ClientID);
			}
		}
		else if(Msg == NETMSG_INPUT)
		{
			CClient::CInput *pInput;
			int64 TagTime;
			int64 Now = time_get();

			m_aClients[ClientID].m_LastAckedSnapshot = Unpacker.GetInt();
			int IntendedTick = Unpacker.GetInt();
			int Size = Unpacker.GetInt();

			// check for errors
			if(Unpacker.Error() || Size/4 > MAX_INPUT_SIZE)
				return;

			if(m_aClients[ClientID].m_LastAckedSnapshot > 0)
				m_aClients[ClientID].m_SnapRate = CClient::SNAPRATE_FULL;

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

			m_aClients[ClientID].m_LastInputTick = IntendedTick;

			pInput = &m_aClients[ClientID].m_aInputs[m_aClients[ClientID].m_CurrentInput];

			if(IntendedTick <= Tick())
				IntendedTick = Tick()+1;

			pInput->m_GameTick = IntendedTick;

			for(int i = 0; i < Size/4; i++)
				pInput->m_aData[i] = Unpacker.GetInt();

			int PingCorrection = clamp(Unpacker.GetInt(), 0, 50);
			if(m_aClients[ClientID].m_Snapshots.Get(m_aClients[ClientID].m_LastAckedSnapshot, &TagTime, 0, 0) >= 0)
			{
				m_aClients[ClientID].m_Latency = (int)(((Now-TagTime)*1000)/time_freq());
				m_aClients[ClientID].m_Latency = maximum(0, m_aClients[ClientID].m_Latency - PingCorrection);
			}

			mem_copy(m_aClients[ClientID].m_LatestInput.m_aData, pInput->m_aData, MAX_INPUT_SIZE*sizeof(int));

			m_aClients[ClientID].m_CurrentInput++;
			m_aClients[ClientID].m_CurrentInput %= 200;

			// call the mod with the fresh input data
			if(m_aClients[ClientID].m_State == CClient::STATE_INGAME)
				GameServer()->OnClientDirectInput(ClientID, m_aClients[ClientID].m_LatestInput.m_aData);
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

			if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Unpacker.Error() == 0)
			{
				if(Config()->m_SvRconPassword[0] == 0 && Config()->m_SvRconModPassword[0] == 0)
				{
					if(!m_aClients[ClientID].m_NoRconNote)
					{
						SendRconLine(ClientID, "No rcon password set on server. Set sv_rcon_password and/or sv_rcon_mod_password to enable the remote console.");
						m_aClients[ClientID].m_NoRconNote = true;
					}
				}
				else if(Config()->m_SvRconPassword[0] && str_comp(pPw, Config()->m_SvRconPassword) == 0)
				{
					CMsgPacker Msg(NETMSG_RCON_AUTH_ON, true);
					SendMsg(&Msg, MSGFLAG_VITAL, ClientID);

					m_aClients[ClientID].m_Authed = AUTHED_ADMIN;
					if(m_aClients[ClientID].m_Version >= MIN_MAPLIST_CLIENTVERSION)
						m_aClients[ClientID].m_pMapListEntryToSend = m_pFirstMapEntry;
					SendRconLine(ClientID, "Admin authentication successful. Full remote console access granted.");
					char aAddrStr[NETADDR_MAXSTRSIZE];
					net_addr_str(m_NetServer.ClientAddr(ClientID), aAddrStr, sizeof(aAddrStr), true);
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "ClientID=%d addr=%s authed (admin)", ClientID, aAddrStr);
					dbg_msg("server", "%s", aBuf);
				}
				else if(Config()->m_SvRconModPassword[0] && str_comp(pPw, Config()->m_SvRconModPassword) == 0)
				{
					CMsgPacker Msg(NETMSG_RCON_AUTH_ON, true);
					SendMsg(&Msg, MSGFLAG_VITAL, ClientID);

					m_aClients[ClientID].m_Authed = AUTHED_MOD;
					SendRconLine(ClientID, "Moderator authentication successful. Limited remote console access granted.");
					char aAddrStr[NETADDR_MAXSTRSIZE];
					net_addr_str(m_NetServer.ClientAddr(ClientID), aAddrStr, sizeof(aAddrStr), true);
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "ClientID=%d addr=%s authed (moderator)", ClientID, aAddrStr);
					dbg_msg("server", "%s", aBuf);
				}
				else
				{
					SendRconLine(ClientID, "Wrong password.");
				}
			}
		}
		else if(Msg == NETMSG_PING)
		{
			CMsgPacker Msg(NETMSG_PING_REPLY, true);
			SendMsg(&Msg, 0, ClientID);
		}
		else
		{
			if(Config()->m_Debug)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "strange message ClientID=%d msg=%d data_size=%d", ClientID, Msg, pPacket->m_DataSize);
				dbg_msg("server", "%s", aBuf);
				str_hex(aBuf, sizeof(aBuf), pPacket->m_pData, minimum(pPacket->m_DataSize, 32));
				dbg_msg("server", "%s", aBuf);
			}
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
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_aClients[i].m_State != CClient::STATE_EMPTY)
		{
			if(GameServer()->IsClientPlayer(i))
				PlayerCount++;

			ClientCount++;
		}
	}

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
	if(Config()->m_Password[0])  // password set
		Flags |= SERVERINFO_FLAG_PASSWORD;
	if(GameServer()->TimeScore())
		Flags |= SERVERINFO_FLAG_TIMESCORE;
	pPacker->AddInt(Flags);

	pPacker->AddInt(0);	// server skill level
	pPacker->AddInt(PlayerCount); // num players
	pPacker->AddInt(10); // max players
	pPacker->AddInt(ClientCount); // num clients
	pPacker->AddInt(maximum(ClientCount, GetMaxClients())); // max clients

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
				pPacker->AddInt(GameServer()->IsClientPlayer(i)?0:1); // flag spectator=1, bot=2 (player=0)
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


	m_NetServer.Update();

	// process packets
	while(m_NetServer.Recv(&Packet, &ResponseToken))
	{
		if(Packet.m_Flags&NETSENDFLAG_CONNLESS)
		{
			if(m_Register.RegisterProcessPacket(&Packet, ResponseToken))
				continue;
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

int CServer::GetPort()
{
	return 8303;
}

void CServer::ChangeMap(const char *pMap)
{
	m_MapReload = true;
}

int CServer::LoadMap(const char *pMapName)
{
	char aBuf[IO_MAX_PATH_LENGTH];
	str_format(aBuf, sizeof(aBuf), "maps/%s.map", pMapName);

	// reinit snapshot ids
	m_IDPool.TimeoutIDs();

	// get the sha256 and crc of the map
	static const unsigned char DM1_SHA256[] = {0x49, 0x1a, 0xf1, 0x7a, 0x51, 0x02, 0x14, 0x50, 0x62, 0x70, 0x90, 0x4f, 0x14, 0x7a, 0x4c, 0x30, 0xae, 0x0a, 0x85, 0xb9, 0x1b, 0xb8, 0x54, 0x39, 0x5b, 0xef, 0x8c, 0x39, 0x7f, 0xc0, 0x78, 0xc3};
	mem_copy(m_CurrentMapSha256.data, DM1_SHA256, sizeof(DM1_SHA256));
	m_CurrentMapCrc = 1683261464; // should print "crc is 64548818"

	char aSha256[SHA256_MAXSTRSIZE];
	sha256_str(m_CurrentMapSha256, aSha256, sizeof(aSha256));
	char aBufMsg[256];
	str_format(aBufMsg, sizeof(aBufMsg), "%s sha256 is %s", aBuf, aSha256);
	dbg_msg("server", "%s", aBufMsg);
	str_format(aBufMsg, sizeof(aBufMsg), "%s crc is %08x", aBuf, m_CurrentMapCrc);
	dbg_msg("server", "%s", aBufMsg);

	str_copy(m_aCurrentMap, pMapName, sizeof(m_aCurrentMap));

	// load complete map into memory for download
	{
		// IOHANDLE File = Storage()->OpenFile(aBuf, IOFLAG_READ, IStorage::TYPE_ALL);
		// if(!File)
		// {
		// 	dbg_msg("server", "TODO: load map into memory");
		// 	return 1;
		// }
		// m_CurrentMapSize = (int)io_length(File);
		// if(m_pCurrentMapData)
		// 	mem_free(m_pCurrentMapData);
		// m_pCurrentMapData = (unsigned char *)mem_alloc(m_CurrentMapSize);
		// io_read(File, m_pCurrentMapData, m_CurrentMapSize);
		// io_close(File);
	}
	return 1;
}

void CServer::InitRegister(CNetServer *pNetServer, CConfig *pConfig)
{
	m_Register.Init(pNetServer, pConfig);
}

void CServer::InitInterfaces(IKernel *pKernel)
{
	m_pConfig = pKernel->RequestInterface<IConfigManager>()->Values();
	m_pGameServer = pKernel->RequestInterface<IGameServer>();
}

int CServer::Run(bool shutdown)
{
	//
	m_PrintCBIndex = 1;

	InitMapList();

	// load map
	if(!LoadMap(GetMapName()))
	{
		dbg_msg("server", "failed to load map. mapname='%s'", GetMapName());
		Free();
		return -1;
	}
	m_MapChunksPerRequest = 1;

	// start server
	NETADDR BindAddr;
	if(Config()->m_Bindaddr[0] && net_host_lookup(Config()->m_Bindaddr, &BindAddr, NETTYPE_ALL) == 0)
	{
		// sweet!
		BindAddr.type = NETTYPE_ALL;
		BindAddr.port = GetPort();
	}
	else
	{
		mem_zero(&BindAddr, sizeof(BindAddr));
		BindAddr.type = NETTYPE_ALL;
		BindAddr.port = GetPort();
	}

	if(!m_NetServer.Open(BindAddr, Config(), Kernel()->RequestInterface<IEngine>(),
		GetMaxClients(), GetMaxClientsPerIP(), NewClientCallback, DelClientCallback, this))
	{
		dbg_msg("server", "couldn't open socket. port %d might already be in use", GetPort());
		Free();
		return -1;
	}

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "server name is '%s'", GetServerName());
	dbg_msg("server", "%s", aBuf);

	GameServer()->OnInit();
	str_format(aBuf, sizeof(aBuf), "netversion %s", GameServer()->NetVersion());
	dbg_msg("server", "%s", aBuf);
	if(str_comp(GameServer()->NetVersionHashUsed(), GameServer()->NetVersionHashReal()))
	{
		dbg_msg("server", "WARNING: netversion hash differs");
	}
	str_format(aBuf, sizeof(aBuf), "game version %s", GameServer()->Version());
	dbg_msg("server", "%s", aBuf);

	if(m_GeneratedRconPassword)
	{
		dbg_msg("server", "+-------------------------+");
		dbg_msg("server", "| rcon password: '%s' |", Config()->m_SvRconPassword);
		dbg_msg("server", "+-------------------------+");
	}

	// start game
	{
		m_GameStartTime = time_get();

		while(m_RunServer)
		{
			// load new map
			if(m_MapReload || m_CurrentGameTick >= 0x6FFFFFFF) //	force reload to make sure the ticks stay within a valid range
			{
				m_MapReload = false;

				// load map
				if(LoadMap(GetMapName()))
				{
					// new map loaded
					bool aSpecs[MAX_CLIENTS];
					for(int c = 0; c < MAX_CLIENTS; c++)
						aSpecs[c] = GameServer()->IsClientSpectator(c);

					GameServer()->OnShutdown();

					for(int c = 0; c < MAX_CLIENTS; c++)
					{
						if(m_aClients[c].m_State <= CClient::STATE_AUTH)
							continue;

						SendMap(c);
						m_aClients[c].Reset();
						m_aClients[c].m_State = aSpecs[c] ? CClient::STATE_CONNECTING_AS_SPEC : CClient::STATE_CONNECTING;
					}

					m_GameStartTime = time_get();
					m_CurrentGameTick = 0;
					Kernel()->ReregisterInterface(GameServer());
					GameServer()->OnInit();
				}
				else
				{
					str_format(aBuf, sizeof(aBuf), "failed to load map. mapname='%s'", GetMapName());
					dbg_msg("server", "%s", aBuf);
				}
			}

			int64 Now = time_get();
			bool NewTicks = false;
			bool ShouldSnap = false;
			while(Now > TickStartTime(m_CurrentGameTick+1))
			{
				m_CurrentGameTick++;
				NewTicks = true;
				if((m_CurrentGameTick%2) == 0)
					ShouldSnap = true;

				// apply new input
				for(int c = 0; c < MAX_CLIENTS; c++)
				{
					if(m_aClients[c].m_State == CClient::STATE_EMPTY)
						continue;
					for(int i = 0; i < 200; i++)
					{
						if(m_aClients[c].m_aInputs[i].m_GameTick == Tick())
						{
							if(m_aClients[c].m_State == CClient::STATE_INGAME)
								GameServer()->OnClientPredictedInput(c, m_aClients[c].m_aInputs[i].m_aData);
							break;
						}
					}
				}

				GameServer()->OnTick();
			}

			// snap game
			if(NewTicks)
			{
				if(Config()->m_SvHighBandwidth || ShouldSnap)
					DoSnapshot();

				UpdateClientMapListEntries();
			}

			// master server stuff
			m_Register.RegisterUpdate(m_NetServer.NetType());

			PumpNetwork();

			// wait for incoming data
			m_NetServer.Wait(clamp(int((TickStartTime(m_CurrentGameTick+1)-time_get())*1000/time_freq()), 1, 1000/SERVER_TICK_SPEED/2));

			if(InterruptSignaled)
			{
				dbg_msg("server", "interrupted");
				break;
			}
			if(shutdown)
				m_RunServer = false;
		}
	}
	// disconnect all clients on shutdown
	m_NetServer.Close(m_aShutdownReason);

	GameServer()->OnShutdown();
	Free();

	return 0;
}

void CServer::Free()
{
	if(m_pCurrentMapData)
	{
		mem_free(m_pCurrentMapData);
		m_pCurrentMapData = 0;
	}

	if(m_pMapListHeap)
	{
		delete m_pMapListHeap;
		m_pMapListHeap = 0;
	}
}

struct CSubdirCallbackUserdata
{
	CServer *m_pServer;
	char m_aName[256];
	bool m_StandardOnly;
};

int CServer::MapListEntryCallback(const char *pFilename, int IsDir, int DirType, void *pUser)
{
	return 0;
}

void CServer::InitMapList()
{
	m_pMapListHeap = new CHeap();

	CSubdirCallbackUserdata Userdata;
	Userdata.m_StandardOnly = false;

	Userdata.m_pServer = this;
	str_copy(Userdata.m_aName, "", sizeof(Userdata.m_aName));
	dbg_msg("server", "%d maps added to maplist", m_NumMapEntries);
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


void HandleSigIntTerm(int Param)
{
	InterruptSignaled = 1;

	// Exit the next time a signal is received
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}

int main(int argc, const char **argv) // ignore_convention
{
	cmdline_fix(&argc, &argv);
	bool shutdown = false;
	for(int i = 1; i < argc; i++) // ignore_convention
	{
#if defined(CONF_FAMILY_WINDOWS)
		if(str_comp("-s", argv[i]) == 0 || str_comp("--silent", argv[i]) == 0) // ignore_convention
		{
			ShowWindow(GetConsoleWindow(), SW_HIDE);
			break;
		}
#endif
		if(str_comp("shutdown", argv[i]) == 0) // ignore_convention
		{
			shutdown = true;
		}
	}

	if(secure_random_init() != 0)
	{
		dbg_msg("secure", "could not initialize secure RNG");
		return -1;
	}

	signal(SIGINT, HandleSigIntTerm);
	signal(SIGTERM, HandleSigIntTerm);

	CServer *pServer = CreateServer();
	IKernel *pKernel = IKernel::Create();

	// create the components
	int FlagMask = CFGFLAG_SERVER|CFGFLAG_ECON;
	IEngine *pEngine = CreateEngine("Teeworlds_Server");
	IGameServer *pGameServer = CreateGameServer();
	IConfigManager *pConfigManager = CreateConfigManager();

	pServer->InitRegister(&pServer->m_NetServer, pConfigManager->Values());

	{
		bool RegisterFail = false;

		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pServer); // register as both
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pEngine);
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pGameServer);
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pConfigManager);

		if(RegisterFail)
			return -1;
	}

	pEngine->Init();
	pConfigManager->Init(FlagMask);

	pServer->InitInterfaces(pKernel);

	// restore empty config strings to their defaults
	pConfigManager->RestoreStrings();

	pServer->InitRconPasswordIfUnset();

	// run the server
	dbg_msg("server", "starting...");
	int Ret = pServer->Run(shutdown);

	// free
	delete pServer;
	delete pKernel;
	delete pEngine;
	delete pGameServer;
	delete pConfigManager;

	secure_random_uninit();
	cmdline_free(argc, argv);
	return Ret;
}
