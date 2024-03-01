/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <algorithm>

#include <stdarg.h>

#include <base/math.h>
#include <base/system.h>

#include <engine/client.h>
#include <engine/engine.h>

#include <engine/shared/compression.h>
#include <engine/shared/network.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>
#include <engine/shared/ringbuffer.h>
#include <engine/shared/snapshot.h>

#include <game/version.h>

#include "client.h"

#if defined(CONF_FAMILY_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#ifdef main
#undef main
#endif

void CGraph::Init(float Min, float Max)
{
	m_MinRange = m_Min = Min;
	m_MaxRange = m_Max = Max;
	m_Index = 0;
}

void CGraph::Scale()
{
	m_Min = m_MinRange;
	m_Max = m_MaxRange;
	for(int i = 0; i < MAX_VALUES; i++)
	{
		if(m_aValues[i] > m_Max)
			m_Max = m_aValues[i];
		else if(m_aValues[i] < m_Min)
			m_Min = m_aValues[i];
	}
}

void CGraph::Add(float v, float r, float g, float b)
{
	m_Index = (m_Index+1)%MAX_VALUES;
	m_aValues[m_Index] = v;
	m_aColors[m_Index][0] = r;
	m_aColors[m_Index][1] = g;
	m_aColors[m_Index][2] = b;
}


void CSmoothTime::Init(int64 Target)
{
	m_Snap = time_get();
	m_Current = Target;
	m_Target = Target;
	m_aAdjustSpeed[0] = 0.3f;
	m_aAdjustSpeed[1] = 0.3f;
	m_Graph.Init(0.0f, 0.5f);
	m_SpikeCounter = 0;
	m_BadnessScore = -100;
}

void CSmoothTime::SetAdjustSpeed(int Direction, float Value)
{
	m_aAdjustSpeed[Direction] = Value;
}

int64 CSmoothTime::Get(int64 Now)
{
	int64 c = m_Current + (Now - m_Snap);
	int64 t = m_Target + (Now - m_Snap);

	// it's faster to adjust upward instead of downward
	// we might need to adjust these abit

	float AdjustSpeed = m_aAdjustSpeed[0];
	if(t > c)
		AdjustSpeed = m_aAdjustSpeed[1];

	float a = ((Now-m_Snap)/(float)time_freq()) * AdjustSpeed;
	if(a > 1.0f)
		a = 1.0f;

	int64 r = c + (int64)((t-c)*a);

	m_Graph.Add(a+0.5f,1,1,1);

	return r;
}

void CSmoothTime::UpdateInt(int64 Target)
{
	int64 Now = time_get();
	m_Current = Get(Now);
	m_Snap = Now;
	m_Target = Target;
}

void CSmoothTime::Update(CGraph *pGraph, int64 Target, int TimeLeft, int AdjustDirection)
{
	int UpdateTimer = 1;

	if(TimeLeft < 0)
	{
		int IsSpike = 0;
		if(TimeLeft < -50)
		{
			IsSpike = 1;

			m_SpikeCounter += 5;
			if(m_SpikeCounter > 50)
				m_SpikeCounter = 50;
		}

		if(IsSpike && m_SpikeCounter < 15)
		{
			// ignore this ping spike
			UpdateTimer = 0;
			pGraph->Add(TimeLeft, 1,1,0.3f); // yellow
			m_BadnessScore += 10;
		}
		else
		{
			pGraph->Add(TimeLeft, 1,0.3f,0.3f); // red
			m_BadnessScore += 50;
			if(m_aAdjustSpeed[AdjustDirection] < 30.0f)
				m_aAdjustSpeed[AdjustDirection] *= 2.0f;
		}
	}
	else
	{
		if(m_SpikeCounter)
			m_SpikeCounter--;

		pGraph->Add(TimeLeft, 0.3f,1,0.3f); // green

		m_aAdjustSpeed[AdjustDirection] *= 0.95f;
		if(m_aAdjustSpeed[AdjustDirection] < 2.0f)
			m_aAdjustSpeed[AdjustDirection] = 2.0f;
	}

	if(UpdateTimer)
		UpdateInt(Target);

	m_BadnessScore -= 1+m_BadnessScore/100;
}

CClient::CClient()
{
	m_pGameClient = 0;

	m_WindowMustRefocus = 0;
	m_SnapCrcErrors = 0;

	m_AckGameTick = -1;
	m_CurrentRecvTick = 0;
	m_RconAuthed = 0;

	// version-checking
	m_aVersionStr[0] = '0';
	m_aVersionStr[1] = 0;

	// pinging
	m_PingStartTime = 0;

	//
	m_aCurrentMap[0] = 0;
	m_CurrentMapCrc = 0;

	//
	m_aCmdConnect[0] = 0;

	// map download
	m_aMapdownloadFilename[0] = 0;
	m_aMapdownloadFilenameTemp[0] = 0;
	m_aMapdownloadName[0] = 0;
	m_MapdownloadFileTemp = 0;
	m_MapdownloadChunk = 0;
	m_MapdownloadCrc = 0;
	m_MapdownloadAmount = -1;
	m_MapdownloadTotalsize = -1;

	m_CurrentInput = 0;

	m_aServerAddressStr[0] = 0;
	m_aServerPassword[0] = 0;

	mem_zero(m_aSnapshots, sizeof(m_aSnapshots));
	m_SnapshotStorage.Init();
	m_ReceivedSnapshots = 0;
}

// ----- send functions -----
int CClient::SendMsg(CMsgPacker *pMsg, int Flags)
{
	CNetChunk Packet;

	mem_zero(&Packet, sizeof(CNetChunk));
	Packet.m_ClientID = 0;
	Packet.m_pData = pMsg->Data();
	Packet.m_DataSize = pMsg->Size();

	if(Flags&MSGFLAG_VITAL)
		Packet.m_Flags |= NETSENDFLAG_VITAL;
	if(Flags&MSGFLAG_FLUSH)
		Packet.m_Flags |= NETSENDFLAG_FLUSH;

	if(Flags&MSGFLAG_RECORD)
	{
		// dbg_msg("send", "flag record");
	}

	if(!(Flags&MSGFLAG_NOSEND))
		m_NetClient.Send(&Packet);
	return 0;
}

void CClient::SendInfo()
{
	// restore password of favorite if possible
	str_copy(m_aServerPassword, "password!!!", sizeof(m_aServerPassword));

	CMsgPacker Msg(NETMSG_INFO, true);
	Msg.AddString(GameClient()->NetVersion(), 128);
	Msg.AddString(m_aServerPassword, 128);
	Msg.AddInt(GameClient()->ClientVersion());
	SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);
}


void CClient::SendEnterGame()
{
	CMsgPacker Msg(NETMSG_ENTERGAME, true);
	SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);
}

void CClient::SendReady()
{
	CMsgPacker Msg(NETMSG_READY, true);
	SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);
}

void CClient::RconAuth(const char *pName, const char *pPassword)
{
	if(RconAuthed())
		return;

	CMsgPacker Msg(NETMSG_RCON_AUTH, true);
	Msg.AddString(pPassword, 32);
	SendMsg(&Msg, MSGFLAG_VITAL);
}

void CClient::Rcon(const char *pCmd)
{
	CMsgPacker Msg(NETMSG_RCON_CMD, true);
	Msg.AddString(pCmd, 256);
	SendMsg(&Msg, MSGFLAG_VITAL);
}

void CClient::SendInput()
{
	int64 Now = time_get();

	// fetch input
	int Size = GameClient()->OnSnapInput(m_aInputs[m_CurrentInput].m_aData);

	if(!Size)
		return;

	int AckGameTick = 10;
	int PredTick = 10;

	// pack input
	CMsgPacker Msg(NETMSG_INPUT, true);
	Msg.AddInt(AckGameTick);
	Msg.AddInt(PredTick);
	Msg.AddInt(Size);

	m_aInputs[m_CurrentInput].m_Tick = PredTick;
	m_aInputs[m_CurrentInput].m_PredictedTime = m_PredictedTime.Get(Now);
	m_aInputs[m_CurrentInput].m_Time = Now;

	// pack it
	for(int i = 0; i < Size/4; i++)
		Msg.AddInt(m_aInputs[m_CurrentInput].m_aData[i]);

	int PingCorrection = 0;
	int64 TagTime;
	if(m_SnapshotStorage.Get(m_AckGameTick, &TagTime, 0, 0) >= 0)
		PingCorrection = (int)(((Now-TagTime)*1000)/time_freq());
	Msg.AddInt(PingCorrection);

	m_CurrentInput++;
	m_CurrentInput%=200;

	SendMsg(&Msg, MSGFLAG_FLUSH);
}

const char *CClient::LatestVersion() const
{
	return m_aVersionStr;
}

const void *CClient::SnapGetItem(int SnapID, int Index, CSnapItem *pItem) const
{
	dbg_assert(SnapID >= 0 && SnapID < NUM_SNAPSHOT_TYPES, "invalid SnapID");
	const CSnapshotItem *i = m_aSnapshots[SnapID]->m_pAltSnap->GetItem(Index);
	pItem->m_DataSize = m_aSnapshots[SnapID]->m_pAltSnap->GetItemSize(Index);
	pItem->m_Type = i->Type();
	pItem->m_ID = i->ID();
	return i->Data();
}

void CClient::SnapInvalidateItem(int SnapID, int Index)
{
	dbg_assert(SnapID >= 0 && SnapID < NUM_SNAPSHOT_TYPES, "invalid SnapID");
	const CSnapshotItem *i = m_aSnapshots[SnapID]->m_pAltSnap->GetItem(Index);
	if(i)
	{
		if((char *)i < (char *)m_aSnapshots[SnapID]->m_pAltSnap || (char *)i > (char *)m_aSnapshots[SnapID]->m_pAltSnap + m_aSnapshots[SnapID]->m_SnapSize)
			dbg_msg("client", "snap invalidate problem");
		if((char *)i >= (char *)m_aSnapshots[SnapID]->m_pSnap && (char *)i < (char *)m_aSnapshots[SnapID]->m_pSnap + m_aSnapshots[SnapID]->m_SnapSize)
			dbg_msg("client", "snap invalidate problem");
		m_aSnapshots[SnapID]->m_pAltSnap->InvalidateItem(Index);
	}
}

const void *CClient::SnapFindItem(int SnapID, int Type, int ID) const
{
	if(!m_aSnapshots[SnapID])
		return 0x0;

	CSnapshot* pAltSnap = m_aSnapshots[SnapID]->m_pAltSnap;
	int Key = (Type<<16)|(ID&0xffff);
	int Index = pAltSnap->GetItemIndex(Key);
	if(Index != -1)
		return pAltSnap->GetItem(Index)->Data();

	return 0x0;
}

int CClient::SnapNumItems(int SnapID) const
{
	dbg_assert(SnapID >= 0 && SnapID < NUM_SNAPSHOT_TYPES, "invalid SnapID");
	if(!m_aSnapshots[SnapID])
		return 0;
	return m_aSnapshots[SnapID]->m_pSnap->NumItems();
}

void *CClient::SnapNewItem(int Type, int ID, int Size)
{
	dbg_assert(Type >= 0 && Type <=0xffff, "incorrect type");
	dbg_assert(ID >= 0 && ID <=0xffff, "incorrect id");
	return ID < 0 ? 0 : m_DemoRecSnapshotBuilder.NewItem(Type, ID, Size);
}

void CClient::SnapSetStaticsize(int ItemType, int Size)
{
	m_SnapshotDelta.SetStaticsize(ItemType, Size);
}



struct CMastersrvAddr
{
	unsigned char m_aIp[16];
	unsigned char m_aPort[2];
};

void CClient::ProcessConnlessPacket(CNetChunk *pPacket)
{
	static const unsigned char SERVERBROWSE_LIST[] = {255, 255, 255, 255, 'l', 'i', 's', '2'};
	// static const unsigned char VERSIONSRV_VERSION[] = {255, 255, 255, 255, 'v', 'e', 'r', 's'};
	// static const unsigned char VERSIONSRV_GETMAPLIST[] = {255, 255, 255, 255, 'v', 'm', 'l', 'g'};
	// static const unsigned char VERSIONSRV_MAPLIST[] = {255, 255, 255, 255, 'v', 'm', 'l', 's'};

	const char *pConnlessPacket = "CONNLESS_UNKOWN";
	// version server
	// if(m_VersionInfo.m_State == CVersionInfo::STATE_READY && net_addr_comp(&pPacket->m_Address, &m_VersionInfo.m_VersionServeraddr.m_Addr, true) == 0)
	// {
	// 	pConnlessPacket = "CONNLESS_VERSION";
	// 	// version info
	// 	if(pPacket->m_DataSize == (int)(sizeof(VERSIONSRV_VERSION) + sizeof(GAME_RELEASE_VERSION)) &&
	// 		mem_comp(pPacket->m_pData, VERSIONSRV_VERSION, sizeof(VERSIONSRV_VERSION)) == 0)

	// 	{
	// 		char *pVersionData = (char*)pPacket->m_pData + sizeof(VERSIONSRV_VERSION);
	// 		int VersionMatch = !mem_comp(pVersionData, GAME_RELEASE_VERSION, sizeof(GAME_RELEASE_VERSION));

	// 		char aVersion[sizeof(GAME_RELEASE_VERSION)];
	// 		str_copy(aVersion, pVersionData, sizeof(aVersion));

	// 		char aBuf[256];
	// 		str_format(aBuf, sizeof(aBuf), "version does %s (%s)",
	// 			VersionMatch ? "match" : "NOT match",
	// 			aVersion);
	// 		dbg_msg("client/version", "%s", aBuf);

	// 		// assume version is out of date when version-data doesn't match
	// 		if(!VersionMatch)
	// 		{
	// 			str_copy(m_aVersionStr, aVersion, sizeof(m_aVersionStr));
	// 		}

	// 		// request the map version list now
	// 		CNetChunk Packet;
	// 		mem_zero(&Packet, sizeof(Packet));
	// 		Packet.m_ClientID = -1;
	// 		Packet.m_Address = m_VersionInfo.m_VersionServeraddr.m_Addr;
	// 		Packet.m_pData = VERSIONSRV_GETMAPLIST;
	// 		Packet.m_DataSize = sizeof(VERSIONSRV_GETMAPLIST);
	// 		Packet.m_Flags = NETSENDFLAG_CONNLESS;
	// 		m_ContactClient.Send(&Packet);
	// 	}

	// 	// map version list
	// 	if(pPacket->m_DataSize >= (int)sizeof(VERSIONSRV_MAPLIST) &&
	// 		mem_comp(pPacket->m_pData, VERSIONSRV_MAPLIST, sizeof(VERSIONSRV_MAPLIST)) == 0)
	// 	{
	// 		// int Size = pPacket->m_DataSize-sizeof(VERSIONSRV_MAPLIST);
	// 		// int Num = Size/sizeof(CMapVersion);
	// 		pConnlessPacket = "VERSIONSRV_MAPLIST";
	// 	}
	// }

	// server list from master server
	if(pPacket->m_DataSize >= (int)sizeof(SERVERBROWSE_LIST) &&
		mem_comp(pPacket->m_pData, SERVERBROWSE_LIST, sizeof(SERVERBROWSE_LIST)) == 0)
	{
		pConnlessPacket = "SERVERBROWSE_LIST";
		dbg_msg("network_in", "WE GOT A SERVER LIST SERVERBROWSE_LIST");

		int Size = pPacket->m_DataSize-sizeof(SERVERBROWSE_LIST);
		int Num = Size/sizeof(CMastersrvAddr);
		CMastersrvAddr *pAddrs = (CMastersrvAddr *)((char*)pPacket->m_pData+sizeof(SERVERBROWSE_LIST));
		for(int i = 0; i < Num; i++)
		{
			NETADDR Addr;

			static unsigned char s_aIPV4Mapping[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};

			// copy address
			if(!mem_comp(s_aIPV4Mapping, pAddrs[i].m_aIp, sizeof(s_aIPV4Mapping)))
			{
				mem_zero(&Addr, sizeof(Addr));
				Addr.type = NETTYPE_IPV4;
				Addr.ip[0] = pAddrs[i].m_aIp[12];
				Addr.ip[1] = pAddrs[i].m_aIp[13];
				Addr.ip[2] = pAddrs[i].m_aIp[14];
				Addr.ip[3] = pAddrs[i].m_aIp[15];
			}
			else
			{
				Addr.type = NETTYPE_IPV6;
				mem_copy(Addr.ip, pAddrs[i].m_aIp, sizeof(Addr.ip));
			}
			Addr.port = (pAddrs[i].m_aPort[0]<<8) | pAddrs[i].m_aPort[1];
		}
	}
	dbg_msg("conless", "msg %s", pConnlessPacket);
}

void CClient::ProcessServerPacket(CNetChunk *pPacket)
{
	CUnpacker Unpacker;
	Unpacker.Reset(pPacket->m_pData, pPacket->m_DataSize);

	// unpack msgid and system flag
	int Msg = Unpacker.GetInt();
	int Sys = Msg&1;
	Msg >>= 1;

	if(Unpacker.Error())
		return;

	if(Sys)
	{
		// system message
		if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_MAP_CHANGE)
		{
			const char *pMap = Unpacker.GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
			int MapCrc = Unpacker.GetInt();
			int MapSize = Unpacker.GetInt();
			int MapChunkNum = Unpacker.GetInt();
			int MapChunkSize = Unpacker.GetInt();
			if(Unpacker.Error())
				return;
			const char *pError = 0;

			// protect the player from nasty map names
			for(int i = 0; pMap[i]; i++)
			{
				if(pMap[i] == '/' || pMap[i] == '\\')
					pError = "strange character in map name";
			}

			if(MapSize <= 0)
				pError = "invalid map size";

			if(pError)
			{
				dbg_msg("client", "would disconnect because of error=%s", pError);
			}
			else
			{
				pError = 0;

				if(!pError)
				{
					dbg_msg("client/network", "loading done");
					SendReady();
				}
				else
				{
					// start map download

					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "starting to download map to '%s'", m_aMapdownloadFilenameTemp);
					dbg_msg("client/network", "%s", aBuf);

					str_copy(m_aMapdownloadName, pMap, sizeof(m_aMapdownloadName));
					m_MapdownloadChunk = 0;
					m_MapdownloadChunkNum = MapChunkNum;
					m_MapDownloadChunkSize = MapChunkSize;
					m_MapdownloadCrc = MapCrc;
					m_MapdownloadTotalsize = MapSize;
					m_MapdownloadAmount = 0;

					// request first chunk package of map data
					CMsgPacker Msg(NETMSG_REQUEST_MAP_DATA, true);
					SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);

					dbg_msg("client/network", "requested first chunk package");
				}
			}
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_MAP_DATA)
		{
			if(!m_MapdownloadFileTemp)
				return;

			int Size = minimum(m_MapDownloadChunkSize, m_MapdownloadTotalsize-m_MapdownloadAmount);
			const unsigned char *pData = Unpacker.GetRaw(Size);
			if(Unpacker.Error())
				return;

			io_write(m_MapdownloadFileTemp, pData, Size);
			++m_MapdownloadChunk;
			m_MapdownloadAmount += Size;

			if(m_MapdownloadAmount == m_MapdownloadTotalsize)
			{
				// map download complete
				dbg_msg("client/network", "download complete, loading map");

				if(m_MapdownloadFileTemp)
					io_close(m_MapdownloadFileTemp);
				m_MapdownloadFileTemp = 0;
				m_MapdownloadAmount = 0;
				m_MapdownloadTotalsize = -1;

				dbg_msg("client/network", "loading done");
				SendReady();
			}
			else if(m_MapdownloadChunk%m_MapdownloadChunkNum == 0)
			{
				// request next chunk package of map data
				CMsgPacker Msg(NETMSG_REQUEST_MAP_DATA, true);
				SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);

				dbg_msg("client/network", "requested next chunk package");
			}
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_SERVERINFO)
		{
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_CON_READY)
		{
			// GameClient()->SendStartInfo();
		}
		else if(Msg == NETMSG_PING)
		{
			CMsgPacker Msg(NETMSG_PING_REPLY, true);
			SendMsg(&Msg, 0);
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_RCON_CMD_ADD)
		{
			const char *pName = Unpacker.GetString(CUnpacker::SANITIZE_CC);
			const char *pHelp = Unpacker.GetString(CUnpacker::SANITIZE_CC);
			const char *pParams = Unpacker.GetString(CUnpacker::SANITIZE_CC);
			if(Unpacker.Error() == 0)
				dbg_msg("rcon", "cmd add pName=%s, pParams=%s, pHelp=%s", pName, pParams, pHelp);
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_RCON_CMD_REM)
		{
			const char *pName = Unpacker.GetString(CUnpacker::SANITIZE_CC);
			if(Unpacker.Error() == 0)
				dbg_msg("rcon", "cmd rem name=%s", pName);
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_MAPLIST_ENTRY_ADD)
		{
			const char *pName = Unpacker.GetString(CUnpacker::SANITIZE_CC);
			if(Unpacker.Error() == 0)
				dbg_msg("rcon", "NETMSG_MAPLIST_ENTRY_ADD name=%s", pName);
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_MAPLIST_ENTRY_REM)
		{
			const char *pName = Unpacker.GetString(CUnpacker::SANITIZE_CC);
				dbg_msg("rcon", "NETMSG_MAPLIST_ENTRY_REM name=%s", pName);
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_RCON_AUTH_ON)
		{
			m_RconAuthed = 1;
			m_UseTempRconCommands = 1;
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_RCON_AUTH_OFF)
		{
			m_RconAuthed = 0;
			m_UseTempRconCommands = 0;
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_RCON_LINE)
		{
			const char *pLine = Unpacker.GetString();
			if(Unpacker.Error() == 0)
				GameClient()->OnRconLine(pLine);
		}
		else if(Msg == NETMSG_PING_REPLY)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "latency %.2f", (time_get() - m_PingStartTime)*1000 / (float)time_freq());
			dbg_msg("client/network", "%s", aBuf);
		}
		else if(Msg == NETMSG_INPUTTIMING)
		{
			int InputPredTick = Unpacker.GetInt();
			int TimeLeft = Unpacker.GetInt();

			// adjust our prediction time
			int64 Target = 0;
			for(int k = 0; k < 200; k++)
			{
				if(m_aInputs[k].m_Tick == InputPredTick)
				{
					Target = m_aInputs[k].m_PredictedTime + (time_get() - m_aInputs[k].m_Time);
					Target = Target - (int64)(((TimeLeft-PREDICTION_MARGIN)/1000.0f)*time_freq());
					break;
				}
			}

			if(Target)
				m_PredictedTime.Update(&m_InputtimeMarginGraph, Target, TimeLeft, 1);
		}
		else if(Msg == NETMSG_SNAP || Msg == NETMSG_SNAPSINGLE || Msg == NETMSG_SNAPEMPTY)
		{
			int NumParts = 1;
			int Part = 0;
			int GameTick = Unpacker.GetInt();
			int DeltaTick = GameTick-Unpacker.GetInt();
			int PartSize = 0;
			int Crc = 0;
			int CompleteSize = 0;
			const char *pData = 0;

			if(Msg == NETMSG_SNAP)
			{
				NumParts = Unpacker.GetInt();
				Part = Unpacker.GetInt();
			}

			if(Msg != NETMSG_SNAPEMPTY)
			{
				Crc = Unpacker.GetInt();
				PartSize = Unpacker.GetInt();
			}

			pData = (const char *)Unpacker.GetRaw(PartSize);

			if(Unpacker.Error() || NumParts < 1 || NumParts > CSnapshot::MAX_PARTS || Part < 0 || Part >= NumParts || PartSize < 0 || PartSize > MAX_SNAPSHOT_PACKSIZE)
				return;

			if(GameTick >= m_CurrentRecvTick)
			{
				if(GameTick != m_CurrentRecvTick)
				{
					m_SnapshotParts = 0;
					m_CurrentRecvTick = GameTick;
				}

				// TODO: clean this up abit
				mem_copy((char*)m_aSnapshotIncomingData + Part*MAX_SNAPSHOT_PACKSIZE, pData, PartSize);
				m_SnapshotParts |= 1<<Part;

				if(m_SnapshotParts == (unsigned)((1<<NumParts)-1))
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
					m_SnapshotParts = 0;

					// find snapshot that we should use as delta
					Emptysnap.Clear();

					// find delta
					if(DeltaTick >= 0)
					{
						int DeltashotSize = m_SnapshotStorage.Get(DeltaTick, 0, &pDeltaShot, 0);

						if(DeltashotSize < 0)
						{
							// couldn't find the delta snapshots that the server used
							// to compress this snapshot. force the server to resync
							char aBuf[256];
							str_format(aBuf, sizeof(aBuf), "error, couldn't find the delta snapshot");
							dbg_msg("client", "%s", aBuf);

							// ack snapshot
							// TODO: combine this with the input message
							m_AckGameTick = -1;
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
							return;

						pDeltaData = aTmpBuffer2;
						DeltaSize = IntSize;
					}

					// unpack delta
					SnapSize = m_SnapshotDelta.UnpackDelta(pDeltaShot, pTmpBuffer3, pDeltaData, DeltaSize);
					if(SnapSize < 0)
					{
						char aBuf[64];
						str_format(aBuf, sizeof(aBuf), "delta unpack failed! (%d)", SnapSize);
						dbg_msg("client", "%s", aBuf);
						return;
					}

					if(Msg != NETMSG_SNAPEMPTY && pTmpBuffer3->Crc() != Crc)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "snapshot crc error #%d - tick=%d wantedcrc=%d gotcrc=%d compressed_size=%d delta_tick=%d",
							m_SnapCrcErrors, GameTick, Crc, pTmpBuffer3->Crc(), CompleteSize, DeltaTick);
						dbg_msg("client", "%s", aBuf);

						m_SnapCrcErrors++;
						if(m_SnapCrcErrors > 10)
						{
							// to many errors, send reset
							m_AckGameTick = -1;
							SendInput();
							m_SnapCrcErrors = 0;
						}
						return;
					}
					else
					{
						if(m_SnapCrcErrors)
							m_SnapCrcErrors--;
					}

					// purge old snapshots
					PurgeTick = DeltaTick;
					if(m_aSnapshots[SNAP_PREV] && m_aSnapshots[SNAP_PREV]->m_Tick < PurgeTick)
						PurgeTick = m_aSnapshots[SNAP_PREV]->m_Tick;
					if(m_aSnapshots[SNAP_CURRENT] && m_aSnapshots[SNAP_CURRENT]->m_Tick < PurgeTick)
						PurgeTick = m_aSnapshots[SNAP_CURRENT]->m_Tick;
					m_SnapshotStorage.PurgeUntil(PurgeTick);

					// add new
					m_SnapshotStorage.Add(GameTick, time_get(), SnapSize, pTmpBuffer3, 1);

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
						m_aSnapshots[SNAP_PREV] = m_SnapshotStorage.m_pFirst;
						m_aSnapshots[SNAP_CURRENT] = m_SnapshotStorage.m_pLast;
					}

					// adjust game time
					if(m_ReceivedSnapshots > 2)
					{
						int64 Now = m_GameTime.Get(time_get());
						int64 TickStart = GameTick * time_freq() / SERVER_TICK_SPEED;
						int64 TimeLeft = (TickStart-Now)*1000 / time_freq();
						m_GameTime.Update(&m_GametimeMarginGraph, (GameTick - 1) * time_freq() / SERVER_TICK_SPEED, TimeLeft, 0);
					}

					// ack snapshot
					m_AckGameTick = GameTick;
				}
			}
		}
	}
	else
	{
		if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0)
		{
			// game message
			GameClient()->OnMessage(Msg, &Unpacker);
		}
	}
}

void CClient::PumpNetwork()
{
	m_NetClient.Update();
	// process non-connless packets
	CNetChunk Packet;
	while(m_NetClient.Recv(&Packet))
	{
		if(!(Packet.m_Flags&NETSENDFLAG_CONNLESS))
			ProcessServerPacket(&Packet);
	}

	// process connless packets data
	m_ContactClient.Update();
	while(m_ContactClient.Recv(&Packet))
	{
		if(Packet.m_Flags&NETSENDFLAG_CONNLESS)
			ProcessConnlessPacket(&Packet);
	}
}

void CClient::Update()
{
	PumpNetwork();
}

static CClient *CreateClient()
{
	CClient *pClient = static_cast<CClient *>(mem_alloc(sizeof(CClient)));
	mem_zero(pClient, sizeof(CClient));
	return new(pClient) CClient;
}

int main(int argc, const char **argv) // ignore_convention
{
	CClient *pClient = CreateClient();
	IKernel *pKernel = IKernel::Create();
	pKernel->RegisterInterface(pClient);

	// create the components
	IEngine *pEngine = CreateEngine("Teeworlds");

	{
		bool RegisterFail = false;

		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pEngine);


		RegisterFail = RegisterFail || !pKernel->RegisterInterface(CreateGameClient());

		if(RegisterFail)
			return -1;
	}

	pEngine->Init();

	// free components
	pClient->~CClient();
	mem_free(pClient);
	delete pKernel;
	delete pEngine;

	return 0;
}
