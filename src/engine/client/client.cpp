/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <algorithm>

#include <stdarg.h>

#include <base/math.h>
#include <base/system.h>

#include <engine/client.h>
#include <engine/config.h>
#include <engine/console.h>
#include <engine/engine.h>
#include <engine/keys.h>
#include <engine/map.h>
#include <engine/masterserver.h>
#include <engine/serverbrowser.h>
#include <engine/storage.h>


#include <engine/shared/config.h>
#include <engine/shared/compression.h>
#include <engine/shared/datafile.h>
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

#include <engine/shared/config.h>

#include "contacts.h"
#include "serverbrowser.h"
#include "client.h"

#if defined(CONF_FAMILY_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#include "SDL.h"
#ifdef main
#undef main
#endif

#include <base/dissector/snapshot.h>

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
	m_pMap = 0;
	m_pMapChecker = 0;
	m_pConfigManager = 0;
	m_pConfig = 0;
	m_pConsole = 0;

	m_RenderFrameTime = 0.0001f;
	m_RenderFrameTimeLow = 1.0f;
	m_RenderFrameTimeHigh = 0.0f;
	m_RenderFrames = 0;
	m_LastRenderTime = time_get();
	m_LastCpuTime = time_get();
	m_LastAvgCpuFrameTime = 0;

	m_GameTickSpeed = SERVER_TICK_SPEED;

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
	m_CurrentMapSha256 = SHA256_ZEROED;
	m_CurrentMapCrc = 0;

	//
	m_aCmdConnect[0] = 0;

	// map download
	m_aMapdownloadFilename[0] = 0;
	m_aMapdownloadFilenameTemp[0] = 0;
	m_aMapdownloadName[0] = 0;
	m_MapdownloadFileTemp = 0;
	m_MapdownloadChunk = 0;
	m_MapdownloadSha256 = SHA256_ZEROED;
	m_MapdownloadSha256Present = false;
	m_MapdownloadCrc = 0;
	m_MapdownloadAmount = -1;
	m_MapdownloadTotalsize = -1;

	m_CurrentInput = 0;

	m_State = IClient::STATE_OFFLINE;
	m_aServerAddressStr[0] = 0;
	m_aServerPassword[0] = 0;

	mem_zero(m_aSnapshots, sizeof(m_aSnapshots));
	m_SnapshotStorage.Init();
	m_ReceivedSnapshots = 0;

	m_VersionInfo.m_State = CVersionInfo::STATE_INIT;
}

// ----- send functions -----
int CClient::SendMsg(CMsgPacker *pMsg, int Flags)
{
	CNetChunk Packet;

	if(State() == IClient::STATE_OFFLINE)
		return 0;

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
	const char *pPassword = m_ServerBrowser.GetFavoritePassword(m_aServerAddressStr);
	if(!pPassword)
		pPassword = Config()->m_Password;
	str_copy(m_aServerPassword, pPassword, sizeof(m_aServerPassword));

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

bool CClient::ConnectionProblems() const
{
	return m_NetClient.GotProblems() != 0;
}

int CClient::GetInputtimeMarginStabilityScore()
{
	return m_PredictedTime.GetStabilityScore();
}

void CClient::SendInput()
{
	int64 Now = time_get();

	if(m_PredTick <= 0)
		return;

	// fetch input
	int Size = GameClient()->OnSnapInput(m_aInputs[m_CurrentInput].m_aData);

	if(!Size)
		return;

	// pack input
	CMsgPacker Msg(NETMSG_INPUT, true);
	Msg.AddInt(m_AckGameTick);
	Msg.AddInt(m_PredTick);
	Msg.AddInt(Size);

	m_aInputs[m_CurrentInput].m_Tick = m_PredTick;
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

// TODO: OPT: do this alot smarter!
const int *CClient::GetInput(int Tick) const
{
	int Best = -1;
	for(int i = 0; i < 200; i++)
	{
		if(m_aInputs[i].m_Tick <= Tick && (Best == -1 || m_aInputs[Best].m_Tick < m_aInputs[i].m_Tick))
			Best = i;
	}

	if(Best != -1)
		return (const int *)m_aInputs[Best].m_aData;
	return 0;
}

// ------ state handling -----
void CClient::SetState(int s)
{
	if(m_State == IClient::STATE_QUITING)
		return;

	int Old = m_State;
	if(Config()->m_Debug)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "state change. last=%d current=%d", m_State, s);
		dbg_msg("client", "%s", aBuf);
	}
	m_State = s;
	if(Old != s)
	{
		GameClient()->OnStateChange(m_State, Old);
		if(s == IClient::STATE_ONLINE)
			OnClientOnline();
	}
}

// called when the map is loaded and we should init for a new round
void CClient::OnEnterGame()
{
	// reset input
	int i;
	for(i = 0; i < 200; i++)
		m_aInputs[i].m_Tick = -1;
	m_CurrentInput = 0;

	// reset snapshots
	m_aSnapshots[SNAP_CURRENT] = 0;
	m_aSnapshots[SNAP_PREV] = 0;
	m_SnapshotStorage.PurgeAll();
	m_ReceivedSnapshots = 0;
	m_SnapshotParts = 0;
	m_PredTick = 0;
	m_CurrentRecvTick = 0;
	m_CurGameTick = 0;
	m_PrevGameTick = 0;
}

void CClient::EnterGame()
{
	if(State() == IClient::STATE_DEMOPLAYBACK)
		return;

	if(State() == IClient::STATE_ONLINE)
	{
		// Don't reset everything while already in game.
		return;
	}

	// now we will wait for two snapshots
	// to finish the connection
	SendEnterGame();
	OnEnterGame();
}

void CClient::OnClientOnline()
{
	// store password and server as favorite if configured, if the server was password protected
	CServerInfo Info = {0};
	GetServerInfo(&Info);
	bool ShouldStorePassword = Config()->m_ClSaveServerPasswords == 2 || (Config()->m_ClSaveServerPasswords == 1 && Info.m_Favorite);
	if(m_aServerPassword[0] && ShouldStorePassword && (Info.m_Flags&IServerBrowser::FLAG_PASSWORD))
	{
		m_ServerBrowser.SetFavoritePassword(m_aServerAddressStr, m_aServerPassword);
	}
}

void CClient::Connect(const char *pAddress)
{
	char aBuf[512];
	int Port = 8303;

	Disconnect();

	str_copy(m_aServerAddressStr, pAddress, sizeof(m_aServerAddressStr));

	str_format(aBuf, sizeof(aBuf), "connecting to '%s'", m_aServerAddressStr);
	dbg_msg("client", "%s", aBuf);

	mem_zero(&m_CurrentServerInfo, sizeof(m_CurrentServerInfo));

	if(net_addr_from_str(&m_ServerAddress, m_aServerAddressStr) != 0 && net_host_lookup(m_aServerAddressStr, &m_ServerAddress, m_NetClient.NetType()) != 0)
	{
		str_format(aBuf, sizeof(aBuf), "could not find the address of %s, connecting to localhost", m_aServerAddressStr);
		dbg_msg("client", "%s", aBuf);
		net_host_lookup("localhost", &m_ServerAddress, m_NetClient.NetType());
	}

	m_RconAuthed = 0;
	m_UseTempRconCommands = 0;
	if(m_ServerAddress.port == 0)
		m_ServerAddress.port = Port;
	m_NetClient.Connect(&m_ServerAddress);
	SetState(IClient::STATE_CONNECTING);

	m_InputtimeMarginGraph.Init(-150.0f, 150.0f);
	m_GametimeMarginGraph.Init(-150.0f, 150.0f);
}

void CClient::DisconnectWithReason(const char *pReason)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "disconnecting. reason='%s'", pReason?pReason:"unknown");
	dbg_msg("client", "%s", aBuf);

	// reset password stored in favorites if it's invalid
	if(pReason && str_find_nocase(pReason, "password"))
	{
		const char *pPassword = m_ServerBrowser.GetFavoritePassword(m_aServerAddressStr);
		if(pPassword && str_comp(pPassword, m_aServerPassword) == 0)
			m_ServerBrowser.SetFavoritePassword(m_aServerAddressStr, 0);
	}

	//
	m_RconAuthed = 0;
	m_UseTempRconCommands = 0;
	m_NetClient.Disconnect(pReason);
	SetState(IClient::STATE_OFFLINE);
	m_pMap->Unload();

	// disable all downloads
	m_MapdownloadChunk = 0;
	if(m_MapdownloadFileTemp)
	{
		io_close(m_MapdownloadFileTemp);
		Storage()->RemoveFile(m_aMapdownloadFilenameTemp, IStorage::TYPE_SAVE);
	}
	m_MapdownloadFileTemp = 0;
	m_MapdownloadSha256 = SHA256_ZEROED;
	m_MapdownloadSha256Present = false;
	m_MapdownloadCrc = 0;
	m_MapdownloadTotalsize = -1;
	m_MapdownloadAmount = 0;

	// clear the current server info
	mem_zero(&m_CurrentServerInfo, sizeof(m_CurrentServerInfo));
	mem_zero(&m_ServerAddress, sizeof(m_ServerAddress));
	m_aServerAddressStr[0] = 0;
	m_aServerPassword[0] = 0;

	// clear snapshots
	m_aSnapshots[SNAP_CURRENT] = 0;
	m_aSnapshots[SNAP_PREV] = 0;
	m_ReceivedSnapshots = 0;
}

void CClient::Disconnect()
{
	DisconnectWithReason(0);
}


void CClient::GetServerInfo(CServerInfo *pServerInfo)
{
	mem_copy(pServerInfo, &m_CurrentServerInfo, sizeof(m_CurrentServerInfo));
	m_ServerBrowser.UpdateFavoriteState(pServerInfo);
}

// ---

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


void CClient::DebugRender()
{
}

void CClient::Quit()
{
	SetState(IClient::STATE_QUITING);
}

const char *CClient::ErrorString() const
{
	return m_NetClient.ErrorString();
}

void CClient::Render()
{
	GameClient()->OnRender();
	DebugRender();
}

const char *CClient::LoadMap(const char *pName, const char *pFilename, const SHA256_DIGEST *pWantedSha256, unsigned WantedCrc)
{
	static char aErrorMsg[512];

	SetState(IClient::STATE_LOADING);

	if(!m_pMap->Load(pFilename))
	{
		str_format(aErrorMsg, sizeof(aErrorMsg), "map '%s' not found", pFilename);
		return aErrorMsg;
	}

	if(pWantedSha256 && m_pMap->Sha256() != *pWantedSha256)
	{
		char aSha256[SHA256_MAXSTRSIZE];
		char aWantedSha256[SHA256_MAXSTRSIZE];
		sha256_str(m_pMap->Sha256(), aSha256, sizeof(aSha256));
		sha256_str(*pWantedSha256, aWantedSha256, sizeof(aWantedSha256));
		str_format(aErrorMsg, sizeof(aErrorMsg), "map differs from the server. found = %s wanted = %s", aSha256, aWantedSha256);
		dbg_msg("client", "%s", aErrorMsg);
		m_pMap->Unload();
		return aErrorMsg;
	}

	// get the crc of the map
	if(m_pMap->Crc() != WantedCrc)
	{
		str_format(aErrorMsg, sizeof(aErrorMsg), "map differs from the server. found = %08x wanted = %08x", m_pMap->Crc(), WantedCrc);
		dbg_msg("client", "%s", aErrorMsg);
		m_pMap->Unload();
		return aErrorMsg;
	}

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "loaded map '%s'", pFilename);
	dbg_msg("client", "%s", aBuf);
	m_ReceivedSnapshots = 0;

	str_copy(m_aCurrentMap, pName, sizeof(m_aCurrentMap));
	str_copy(m_aCurrentMapPath, pFilename, sizeof(m_aCurrentMapPath));
	m_CurrentMapSha256 = m_pMap->Sha256();
	m_CurrentMapCrc = m_pMap->Crc();

	return 0x0;
}


static void FormatMapDownloadFilename(const char *pName, const SHA256_DIGEST *pSha256, int Crc, bool Temp, char *pBuffer, int BufferSize)
{
	char aSuffix[32];
	if(Temp)
	{
		str_format(aSuffix, sizeof(aSuffix), ".%d.tmp", pid());
	}
	else
	{
		str_copy(aSuffix, ".map", sizeof(aSuffix));
	}

	if(pSha256)
	{
		char aSha256[SHA256_MAXSTRSIZE];
		sha256_str(*pSha256, aSha256, sizeof(aSha256));
		str_format(pBuffer, BufferSize, "downloadedmaps/%s_%s%s", pName, aSha256, aSuffix);
	}
	else
	{
		str_format(pBuffer, BufferSize, "downloadedmaps/%s_%08x%s", pName, Crc, aSuffix);
	}
}


const char *CClient::LoadMapSearch(const char *pMapName, const SHA256_DIGEST *pWantedSha256, int WantedCrc)
{
	const char *pError = 0;
	char aBuf[512];
	char aWanted[SHA256_MAXSTRSIZE + 16];
	aWanted[0] = 0;
	if(pWantedSha256)
	{
		char aWantedSha256[SHA256_MAXSTRSIZE];
		sha256_str(*pWantedSha256, aWantedSha256, sizeof(aWantedSha256));
		str_format(aWanted, sizeof(aWanted), "sha256=%s ", aWantedSha256);
	}
	str_format(aBuf, sizeof(aBuf), "loading map, map=%s wanted %scrc=%08x", pMapName, aWanted, WantedCrc);
	dbg_msg("client", "%s", aBuf);
	SetState(IClient::STATE_LOADING);

	// try the normal maps folder
	str_format(aBuf, sizeof(aBuf), "maps/%s.map", pMapName);
	pError = LoadMap(pMapName, aBuf, pWantedSha256, WantedCrc);
	if(!pError)
		return pError;

	// try the downloaded maps
	FormatMapDownloadFilename(pMapName, pWantedSha256, WantedCrc, false, aBuf, sizeof(aBuf));
	pError = LoadMap(pMapName, aBuf, pWantedSha256, WantedCrc);
	if(!pError)
		return pError;

	// backward compatibility with old names
	if(pWantedSha256)
	{
		FormatMapDownloadFilename(pMapName, 0, WantedCrc, false, aBuf, sizeof(aBuf));
		pError = LoadMap(pMapName, aBuf, pWantedSha256, WantedCrc);
		if(!pError)
			return pError;
	}

	// search for the map within subfolders
	char aFilename[128];
	str_format(aFilename, sizeof(aFilename), "%s.map", pMapName);
	if(Storage()->FindFile(aFilename, "maps", IStorage::TYPE_ALL, aBuf, sizeof(aBuf)))
		pError = LoadMap(pMapName, aBuf, pWantedSha256, WantedCrc);

	return pError;
}

int CClient::UnpackServerInfo(CUnpacker *pUnpacker, CServerInfo *pInfo, int *pToken)
{
	if(pToken)
		*pToken = pUnpacker->GetInt();
	str_copy(pInfo->m_aVersion, pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES), sizeof(pInfo->m_aVersion));
	str_copy(pInfo->m_aName, pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES), sizeof(pInfo->m_aName));
	str_clean_whitespaces(pInfo->m_aName);
	str_copy(pInfo->m_aHostname, pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES), sizeof(pInfo->m_aHostname));
	if(pInfo->m_aHostname[0] == 0)
		str_copy(pInfo->m_aHostname, pInfo->m_aAddress, sizeof(pInfo->m_aHostname));
	str_copy(pInfo->m_aMap, pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES), sizeof(pInfo->m_aMap));
	str_copy(pInfo->m_aGameType, pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES), sizeof(pInfo->m_aGameType));
	int Flags = pUnpacker->GetInt();
	pInfo->m_Flags = 0;
	if(Flags&SERVERINFO_FLAG_PASSWORD)
		pInfo->m_Flags |= IServerBrowser::FLAG_PASSWORD;
	if(Flags&SERVERINFO_FLAG_TIMESCORE)
		pInfo->m_Flags |= IServerBrowser::FLAG_TIMESCORE;
	pInfo->m_ServerLevel = clamp<int>(pUnpacker->GetInt(), SERVERINFO_LEVEL_MIN, SERVERINFO_LEVEL_MAX);
	pInfo->m_NumPlayers = pUnpacker->GetInt();
	pInfo->m_MaxPlayers = pUnpacker->GetInt();
	pInfo->m_NumClients = pUnpacker->GetInt();
	pInfo->m_MaxClients = pUnpacker->GetInt();
	pInfo->m_NumBotPlayers = 0;
	pInfo->m_NumBotSpectators = 0;

	// don't add invalid info to the server browser list
	if(pInfo->m_NumClients < 0 || pInfo->m_NumClients > pInfo->m_MaxClients || pInfo->m_MaxClients < 0 || pInfo->m_MaxClients > MAX_CLIENTS || pInfo->m_MaxPlayers < pInfo->m_NumPlayers ||
		pInfo->m_NumPlayers < 0 || pInfo->m_NumPlayers > pInfo->m_NumClients || pInfo->m_MaxPlayers < 0 || pInfo->m_MaxPlayers > pInfo->m_MaxClients)
		return -1;
	// drop standard gametype with more than MAX_PLAYERS
	if(pInfo->m_MaxPlayers > MAX_PLAYERS && (str_comp(pInfo->m_aGameType, "DM") == 0 || str_comp(pInfo->m_aGameType, "TDM") == 0 || str_comp(pInfo->m_aGameType, "CTF") == 0 ||
		str_comp(pInfo->m_aGameType, "LTS") == 0 || str_comp(pInfo->m_aGameType, "LMS") == 0))
		return -1;

	// use short version
	if(!pToken)
		return 0;

	int NumPlayers = 0;
	int NumClients = 0;
	for(int i = 0; i < pInfo->m_NumClients; i++)
	{
		str_utf8_copy_num(pInfo->m_aClients[i].m_aName, pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES), sizeof(pInfo->m_aClients[i].m_aName), MAX_NAME_LENGTH);
		str_utf8_copy_num(pInfo->m_aClients[i].m_aClan, pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES), sizeof(pInfo->m_aClients[i].m_aClan), MAX_CLAN_LENGTH);
		pInfo->m_aClients[i].m_Country = pUnpacker->GetInt();
		pInfo->m_aClients[i].m_Score = pUnpacker->GetInt();
		pInfo->m_aClients[i].m_PlayerType = pUnpacker->GetInt()&CServerInfo::CClient::PLAYERFLAG_MASK;

		if(pInfo->m_aClients[i].m_PlayerType&CServerInfo::CClient::PLAYERFLAG_BOT)
		{
			if(pInfo->m_aClients[i].m_PlayerType&CServerInfo::CClient::PLAYERFLAG_SPEC)
				pInfo->m_NumBotSpectators++;
			else
				pInfo->m_NumBotPlayers++;
		}

		NumClients++;
		if(!(pInfo->m_aClients[i].m_PlayerType&CServerInfo::CClient::PLAYERFLAG_SPEC))
			NumPlayers++;
	}
	pInfo->m_NumPlayers = NumPlayers;
	pInfo->m_NumClients = NumClients;

	return 0;
}

bool CompareScore(const CServerInfo::CClient &C1, const CServerInfo::CClient &C2)
{
	if(C1.m_PlayerType&CServerInfo::CClient::PLAYERFLAG_SPEC)
		return false;
	if(C2.m_PlayerType&CServerInfo::CClient::PLAYERFLAG_SPEC)
		return true;
	return C1.m_Score > C2.m_Score;
}

bool CompareTime(const CServerInfo::CClient &C1, const CServerInfo::CClient &C2)
{
	if(C1.m_PlayerType&CServerInfo::CClient::PLAYERFLAG_SPEC)
		return false;
	if(C2.m_PlayerType&CServerInfo::CClient::PLAYERFLAG_SPEC)
		return true;
	if(C1.m_Score < 0)
		return false;
	if(C2.m_Score < 0)
		return true;
	return C1.m_Score < C2.m_Score;
}

inline void SortClients(CServerInfo *pInfo)
{
	std::stable_sort(pInfo->m_aClients, pInfo->m_aClients + pInfo->m_NumClients,
		(pInfo->m_Flags&IServerBrowser::FLAG_TIMESCORE) ? CompareTime : CompareScore);
}

void CClient::ProcessConnlessPacket(CNetChunk *pPacket)
{
	const char *pConnlessPacket = "CONNLESS_UNKOWN";
	// version server
	if(m_VersionInfo.m_State == CVersionInfo::STATE_READY && net_addr_comp(&pPacket->m_Address, &m_VersionInfo.m_VersionServeraddr.m_Addr, true) == 0)
	{
		pConnlessPacket = "CONNLESS_VERSION";
		// version info
		if(pPacket->m_DataSize == (int)(sizeof(VERSIONSRV_VERSION) + sizeof(GAME_RELEASE_VERSION)) &&
			mem_comp(pPacket->m_pData, VERSIONSRV_VERSION, sizeof(VERSIONSRV_VERSION)) == 0)

		{
			char *pVersionData = (char*)pPacket->m_pData + sizeof(VERSIONSRV_VERSION);
			int VersionMatch = !mem_comp(pVersionData, GAME_RELEASE_VERSION, sizeof(GAME_RELEASE_VERSION));

			char aVersion[sizeof(GAME_RELEASE_VERSION)];
			str_copy(aVersion, pVersionData, sizeof(aVersion));

			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "version does %s (%s)",
				VersionMatch ? "match" : "NOT match",
				aVersion);
			dbg_msg("client/version", "%s", aBuf);

			// assume version is out of date when version-data doesn't match
			if(!VersionMatch)
			{
				str_copy(m_aVersionStr, aVersion, sizeof(m_aVersionStr));
			}

			// request the map version list now
			CNetChunk Packet;
			mem_zero(&Packet, sizeof(Packet));
			Packet.m_ClientID = -1;
			Packet.m_Address = m_VersionInfo.m_VersionServeraddr.m_Addr;
			Packet.m_pData = VERSIONSRV_GETMAPLIST;
			Packet.m_DataSize = sizeof(VERSIONSRV_GETMAPLIST);
			Packet.m_Flags = NETSENDFLAG_CONNLESS;
			m_ContactClient.Send(&Packet);
		}

		// map version list
		if(pPacket->m_DataSize >= (int)sizeof(VERSIONSRV_MAPLIST) &&
			mem_comp(pPacket->m_pData, VERSIONSRV_MAPLIST, sizeof(VERSIONSRV_MAPLIST)) == 0)
		{
			int Size = pPacket->m_DataSize-sizeof(VERSIONSRV_MAPLIST);
			int Num = Size/sizeof(CMapVersion);
			m_pMapChecker->AddMaplist((CMapVersion *)((char*)pPacket->m_pData+sizeof(VERSIONSRV_MAPLIST)), Num);
			pConnlessPacket = "VERSIONSRV_MAPLIST";
		}
	}

	// server list from master server
	if(pPacket->m_DataSize >= (int)sizeof(SERVERBROWSE_LIST) &&
		mem_comp(pPacket->m_pData, SERVERBROWSE_LIST, sizeof(SERVERBROWSE_LIST)) == 0)
	{
		pConnlessPacket = "SERVERBROWSE_LIST";
		if(Config()->m_DbgMaster)
		{
			dbg_msg("network_in", "WE GOT A SERVER LIST SERVERBROWSE_LIST");
			print_hex("network_in", "    ", pPacket->m_pData, pPacket->m_DataSize, 12);
		}
		// check for valid master server address
		bool Valid = false;
		for(int i = 0; i < IMasterServer::MAX_MASTERSERVERS; ++i)
		{
			if(m_pMasterServer->IsValid(i))
			{
				NETADDR Addr = m_pMasterServer->GetAddr(i);
				if(net_addr_comp(&pPacket->m_Address, &Addr, true) == 0)
				{
					Valid = true;
					break;
				}
			}
		}
		if(!Valid)
		{
			if(Config()->m_Debug > 2)
				dbg_msg("network_in", "invalid master server list packet");
			return;
		}

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

			m_ServerBrowser.Set(Addr, CServerBrowser::SET_MASTER_ADD, -1, 0x0);

			if(Config()->m_DbgMaster)
			{
				const void *pServerData = (char *)pPacket->m_pData + sizeof(SERVERBROWSE_LIST) + i * sizeof(CMastersrvAddr);

				char aAddrStr[NETADDR_MAXSTRSIZE];
				net_addr_str(&Addr, aAddrStr, sizeof(aAddrStr), true);

				CMastersrvAddr *pAddr = (CMastersrvAddr *)pServerData;
				unsigned short ReparsedPort = (pAddr->m_aPort[0]<<8) | pAddr->m_aPort[1];

				dbg_msg("network_in", "--------------------------------------");
				dbg_msg("network_in", "i=%d", i);
				dbg_msg("network_in", "ip=%s reparsedport=%u", aAddrStr, ReparsedPort);
				print_hex("network_in", "raw: ", pServerData, sizeof(CMastersrvAddr), 12);
				print_hex("network_in", "adr: ", &pAddrs[i], sizeof(CMastersrvAddr), 12);
				dbg_msg("WHAT TJHE FUCL", "IS A JPEG %d", Addr.port);
				print_hex("network_in", "OF A FUCKIN HOGTJ: ", pAddrs[i].m_aPort, 2, 12);
			}
		}
	}

	// server info
	if(pPacket->m_DataSize >= (int)sizeof(SERVERBROWSE_INFO) && mem_comp(pPacket->m_pData, SERVERBROWSE_INFO, sizeof(SERVERBROWSE_INFO)) == 0)
	{
		CUnpacker Up;
		CServerInfo Info = {0};
		Up.Reset((unsigned char*)pPacket->m_pData+sizeof(SERVERBROWSE_INFO), pPacket->m_DataSize-sizeof(SERVERBROWSE_INFO));
		net_addr_str(&pPacket->m_Address, Info.m_aAddress, sizeof(Info.m_aAddress), true);
		int Token;
		if(!UnpackServerInfo(&Up, &Info, &Token) && !Up.Error())
		{
			SortClients(&Info);
			m_ServerBrowser.Set(pPacket->m_Address, CServerBrowser::SET_TOKEN, Token, &Info);
		}
		pConnlessPacket = "CONNLESS_SERVER_INFO";
	}
	if(Config()->m_Debug > 2)
	{
		if(show_addr(&pPacket->m_Address))
			dbg_msg("network_in", "connless packet: %s", pConnlessPacket);
	}
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

	if(m_pConfig->m_Debug == 1) // TODO: remove? this is kinda redundant with print_packet
	{
		char aMsg[512];
		netmsg_to_s(Msg, aMsg, sizeof(aMsg));
		dbg_msg("network_in", "server packet sys=%d msg=%d (%s)", Sys, Msg, aMsg);
	}

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
			const SHA256_DIGEST *pMapSha256 = (const SHA256_DIGEST *)Unpacker.GetRaw(sizeof(*pMapSha256));
			const char *pError = 0;

			// check for valid standard map
			if(!m_pMapChecker->IsMapValid(pMap, pMapSha256, MapCrc, MapSize))
				pError = "invalid standard map";

			// protect the player from nasty map names
			for(int i = 0; pMap[i]; i++)
			{
				if(pMap[i] == '/' || pMap[i] == '\\')
					pError = "strange character in map name";
			}

			if(MapSize <= 0)
				pError = "invalid map size";

			if(pError)
				DisconnectWithReason(pError);
			else
			{
				pError = LoadMapSearch(pMap, pMapSha256, MapCrc);

				if(!pError)
				{
					dbg_msg("client/network", "loading done");
					SendReady();
				}
				else
				{
					if(m_MapdownloadFileTemp)
					{
						io_close(m_MapdownloadFileTemp);
						Storage()->RemoveFile(m_aMapdownloadFilenameTemp, IStorage::TYPE_SAVE);
					}

					// start map download
					FormatMapDownloadFilename(pMap, pMapSha256, MapCrc, false, m_aMapdownloadFilename, sizeof(m_aMapdownloadFilename));
					FormatMapDownloadFilename(pMap, pMapSha256, MapCrc, true, m_aMapdownloadFilenameTemp, sizeof(m_aMapdownloadFilenameTemp));

					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "starting to download map to '%s'", m_aMapdownloadFilenameTemp);
					dbg_msg("client/network", "%s", aBuf);

					str_copy(m_aMapdownloadName, pMap, sizeof(m_aMapdownloadName));
					m_MapdownloadFileTemp = Storage()->OpenFile(m_aMapdownloadFilenameTemp, IOFLAG_WRITE, IStorage::TYPE_SAVE);
					m_MapdownloadChunk = 0;
					m_MapdownloadChunkNum = MapChunkNum;
					m_MapDownloadChunkSize = MapChunkSize;
					m_MapdownloadSha256 = pMapSha256 ? *pMapSha256 : SHA256_ZEROED;
					m_MapdownloadSha256Present = pMapSha256;
					m_MapdownloadCrc = MapCrc;
					m_MapdownloadTotalsize = MapSize;
					m_MapdownloadAmount = 0;

					// request first chunk package of map data
					CMsgPacker Msg(NETMSG_REQUEST_MAP_DATA, true);
					SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);

					if(Config()->m_Debug)
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

				Storage()->RemoveFile(m_aMapdownloadFilename, IStorage::TYPE_SAVE);
				Storage()->RenameFile(m_aMapdownloadFilenameTemp, m_aMapdownloadFilename, IStorage::TYPE_SAVE);

				// load map
				const char *pError = LoadMap(m_aMapdownloadName, m_aMapdownloadFilename, m_MapdownloadSha256Present ? &m_MapdownloadSha256 : 0, m_MapdownloadCrc);
				if(!pError)
				{
					dbg_msg("client/network", "loading done");
					SendReady();
				}
				else
					DisconnectWithReason(pError);
			}
			else if(m_MapdownloadChunk%m_MapdownloadChunkNum == 0)
			{
				// request next chunk package of map data
				CMsgPacker Msg(NETMSG_REQUEST_MAP_DATA, true);
				SendMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);

				if(Config()->m_Debug)
					dbg_msg("client/network", "requested next chunk package");
			}
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_SERVERINFO)
		{
			CServerInfo Info = {0};
			net_addr_str(&pPacket->m_Address, Info.m_aAddress, sizeof(Info.m_aAddress), true);
			if(!UnpackServerInfo(&Unpacker, &Info, 0) && !Unpacker.Error())
			{
				SortClients(&Info);
				mem_copy(&m_CurrentServerInfo, &Info, sizeof(m_CurrentServerInfo));
				m_CurrentServerInfo.m_NetAddr = m_ServerAddress;
			}
		}
		else if((pPacket->m_Flags&NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_CON_READY)
		{
			GameClient()->OnConnected();
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
				dbg_msg("rcon", "cmd add pName=%s, pParams=%s, CFGFLAG_SERVER=%d, pHelp=%s", pName, pParams, CFGFLAG_SERVER, pHelp);
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
			CUnpacker PrintPacker = Unpacker; // create copy that will be modified during print
			print_snapshot(Msg,
				PrintPacker,
				Config(),
				m_ReceivedSnapshots,
				m_SnapshotParts,
				m_SnapshotDelta,
				m_CurrentRecvTick,
				m_SnapshotStorage,
				m_SnapCrcErrors,
				// const class CSmoothTime &GameTime,
				m_aSnapshotIncomingData,
				m_aSnapshots,
				this,
				true);

			int NumParts = 1;
			int Part = 0;
			int GameTick = Unpacker.GetInt();
			int DeltaTick = GameTick-Unpacker.GetInt();
			int PartSize = 0;
			int Crc = 0;
			int CompleteSize = 0;
			const char *pData = 0;

			// we are not allowed to process snapshot yet
			if(State() < IClient::STATE_LOADING)
				return;

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
							if(Config()->m_Debug)
							{
								char aBuf[256];
								str_format(aBuf, sizeof(aBuf), "error, couldn't find the delta snapshot");
								dbg_msg("client", "%s", aBuf);
							}

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
						if(Config()->m_Debug)
						{
							char aBuf[256];
							str_format(aBuf, sizeof(aBuf), "snapshot crc error #%d - tick=%d wantedcrc=%d gotcrc=%d compressed_size=%d delta_tick=%d",
								m_SnapCrcErrors, GameTick, Crc, pTmpBuffer3->Crc(), CompleteSize, DeltaTick);
							dbg_msg("client", "%s", aBuf);
						}

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
						SetState(IClient::STATE_ONLINE);
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

	if(State() != IClient::STATE_DEMOPLAYBACK)
	{
		// check for errors
		if(State() != IClient::STATE_OFFLINE && State() != IClient::STATE_QUITING && m_NetClient.State() == NETSTATE_OFFLINE)
		{
			SetState(IClient::STATE_OFFLINE);
			DisconnectWithReason(m_NetClient.ErrorString());
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "offline error='%s'", m_NetClient.ErrorString());
			dbg_msg("client", "%s", aBuf);
		}

		//
		if(State() == IClient::STATE_CONNECTING && m_NetClient.State() == NETSTATE_ONLINE)
		{
			// we switched to online
			dbg_msg("client", "connected, sending info");
			SetState(IClient::STATE_LOADING);
			SendInfo();
		}
	}

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
	if(State() == IClient::STATE_DEMOPLAYBACK)
	{
		// disconnect on error
		Disconnect();
	}
	else if(State() == IClient::STATE_ONLINE && m_ReceivedSnapshots >= 3)
	{
		// switch snapshot
		int Repredict = 0;
		int64 Freq = time_freq();
		int64 Now = m_GameTime.Get(time_get());
		int64 PredNow = m_PredictedTime.Get(time_get());

		while(1)
		{
			CSnapshotStorage::CHolder *pCur = m_aSnapshots[SNAP_CURRENT];
			int64 TickStart = (pCur->m_Tick) * Freq / SERVER_TICK_SPEED;

			if(TickStart < Now)
			{
				CSnapshotStorage::CHolder *pNext = m_aSnapshots[SNAP_CURRENT]->m_pNext;
				if(pNext)
				{
					m_aSnapshots[SNAP_PREV] = m_aSnapshots[SNAP_CURRENT];
					m_aSnapshots[SNAP_CURRENT] = pNext;

					// set ticks
					m_CurGameTick = m_aSnapshots[SNAP_CURRENT]->m_Tick;
					m_PrevGameTick = m_aSnapshots[SNAP_PREV]->m_Tick;

					if(m_aSnapshots[SNAP_CURRENT] && m_aSnapshots[SNAP_PREV])
					{
						GameClient()->OnNewSnapshot();
						Repredict = 1;
					}
				}
				else
					break;
			}
			else
				break;
		}

		if(m_aSnapshots[SNAP_CURRENT] && m_aSnapshots[SNAP_PREV])
		{
			int64 CurtickStart = m_aSnapshots[SNAP_CURRENT]->m_Tick * Freq / SERVER_TICK_SPEED;
			int64 PrevtickStart = m_aSnapshots[SNAP_PREV]->m_Tick * Freq / SERVER_TICK_SPEED;
			int PrevPredTick = (int)(PredNow * SERVER_TICK_SPEED / Freq);
			int NewPredTick = PrevPredTick+1;

			m_GameIntraTick = (Now - PrevtickStart) / (float)(CurtickStart-PrevtickStart);
			m_GameTickTime = (Now - PrevtickStart) / (float)Freq;

			CurtickStart = NewPredTick * Freq / SERVER_TICK_SPEED;
			PrevtickStart = PrevPredTick * Freq / SERVER_TICK_SPEED;
			m_PredIntraTick = (PredNow - PrevtickStart) / (float)(CurtickStart-PrevtickStart);

			if(NewPredTick < m_aSnapshots[SNAP_PREV]->m_Tick-SERVER_TICK_SPEED || NewPredTick > m_aSnapshots[SNAP_PREV]->m_Tick+SERVER_TICK_SPEED)
			{
				dbg_msg("client", "prediction time reset!");
				m_PredictedTime.Init(m_aSnapshots[SNAP_CURRENT]->m_Tick * Freq / SERVER_TICK_SPEED);
			}

			if(NewPredTick > m_PredTick)
			{
				m_PredTick = NewPredTick;
				Repredict = 1;

				// send input
				SendInput();
			}
		}

		// only do sane predictions
		if(Repredict)
		{
			if(m_PredTick > m_CurGameTick && m_PredTick < m_CurGameTick + SERVER_TICK_SPEED)
				GameClient()->OnPredict();
		}
	}

	// STRESS TEST: join the server again
	if(Config()->m_DbgStress)
	{
		static int64 ActionTaken = 0;
		int64 Now = time_get();
		if(State() == IClient::STATE_OFFLINE)
		{
			if(Now > ActionTaken+time_freq()*2)
			{
				dbg_msg("stress", "reconnecting!");
				Connect(Config()->m_DbgStressServer);
				ActionTaken = Now;
			}
		}
		else
		{
			if(Now > ActionTaken+time_freq()*(10+Config()->m_DbgStress))
			{
				dbg_msg("stress", "disconnecting!");
				Disconnect();
				ActionTaken = Now;
			}
		}
	}

	// pump the network
	PumpNetwork();

	// update the maser server registry
	MasterServer()->Update();

	// update the server browser
	if(Config()->m_Debug < 3)
		m_ServerBrowser.Update();

	GameClient()->OnUpdate();
}

void CClient::VersionUpdate()
{
	if(m_VersionInfo.m_State == CVersionInfo::STATE_INIT)
	{
		Engine()->HostLookup(&m_VersionInfo.m_VersionServeraddr, Config()->m_ClVersionServer, m_ContactClient.NetType());
		m_VersionInfo.m_State = CVersionInfo::STATE_START;
	}
	else if(m_VersionInfo.m_State == CVersionInfo::STATE_START)
	{
		if(m_VersionInfo.m_VersionServeraddr.m_Job.Status() == CJob::STATE_DONE)
		{
			if(m_VersionInfo.m_VersionServeraddr.m_Job.Result() == 0)
			{
				CNetChunk Packet;

				mem_zero(&Packet, sizeof(Packet));

				m_VersionInfo.m_VersionServeraddr.m_Addr.port = VERSIONSRV_PORT;

				Packet.m_ClientID = -1;
				Packet.m_Address = m_VersionInfo.m_VersionServeraddr.m_Addr;
				Packet.m_pData = VERSIONSRV_GETVERSION;
				Packet.m_DataSize = sizeof(VERSIONSRV_GETVERSION);
				Packet.m_Flags = NETSENDFLAG_CONNLESS;

				m_ContactClient.Send(&Packet);
				m_VersionInfo.m_State = CVersionInfo::STATE_READY;
			}
			else
				m_VersionInfo.m_State = CVersionInfo::STATE_ERROR;
		}
	}
}

void CClient::RegisterInterfaces()
{
	Kernel()->RegisterInterface(static_cast<IServerBrowser*>(&m_ServerBrowser));
	Kernel()->RegisterInterface(static_cast<IFriends*>(&m_Friends));
	Kernel()->RegisterInterface(static_cast<IBlacklist*>(&m_Blacklist));
}

void CClient::InitInterfaces()
{
	// fetch interfaces
	m_pEngine = Kernel()->RequestInterface<IEngine>();
	m_pGameClient = Kernel()->RequestInterface<IGameClient>();
	m_pMap = Kernel()->RequestInterface<IEngineMap>();
	m_pMapChecker = Kernel()->RequestInterface<IMapChecker>();
	m_pMasterServer = Kernel()->RequestInterface<IEngineMasterServer>();
	m_pConfigManager = Kernel()->RequestInterface<IConfigManager>();
	m_pConfig = m_pConfigManager->Values();
	m_pStorage = Kernel()->RequestInterface<IStorage>();

	//
	m_ServerBrowser.Init(&m_ContactClient, m_pGameClient->NetVersion());
	m_Friends.Init();
	m_Blacklist.Init();
}

bool CClient::LimitFps()
{
	return true;
}

void CClient::Run()
{
	m_LocalStartTime = time_get();
	m_SnapshotParts = 0;

	// init SDL
	{
		if(SDL_Init(0) < 0)
		{
			dbg_msg("client", "unable to init SDL base: %s", SDL_GetError());
			return;
		}

		atexit(SDL_Quit); // ignore_convention
	}

	// open socket
	{
		NETADDR BindAddr;
		if(Config()->m_Bindaddr[0] && net_host_lookup(Config()->m_Bindaddr, &BindAddr, NETTYPE_ALL) == 0)
		{
			// got bindaddr
			BindAddr.type = NETTYPE_ALL;
		}
		else
		{
			mem_zero(&BindAddr, sizeof(BindAddr));
			BindAddr.type = NETTYPE_ALL;
		}
		if(!m_NetClient.Open(BindAddr, Config(), Console(), Engine(), BindAddr.port ? 0 : NETCREATE_FLAG_RANDOMPORT))
		{
			dbg_msg("client", "couldn't open socket(net)");
			return;
		}
		BindAddr.port = 0;
		if(!m_ContactClient.Open(BindAddr, Config(), Console(), Engine(), 0))
		{
			dbg_msg("client", "couldn't open socket(contact)");
			return;
		}
	}


	// start refreshing addresses while we load
	MasterServer()->RefreshAddresses(m_ContactClient.NetType());

	GameClient()->OnInit();

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "netversion %s", GameClient()->NetVersion());
	dbg_msg("client", "%s", aBuf);
	if(str_comp(GameClient()->NetVersionHashUsed(), GameClient()->NetVersionHashReal()))
	{
		dbg_msg("client", "WARNING: netversion hash differs");
	}
	str_format(aBuf, sizeof(aBuf), "game version %s", GameClient()->Version());
	dbg_msg("client", "%s", aBuf);

	//
	m_FpsGraph.Init(0.0f, 120.0f);

	while (1)
	{
		//
		VersionUpdate();

		// handle pending connects
		if(m_aCmdConnect[0])
		{
			Connect(m_aCmdConnect);
			m_aCmdConnect[0] = 0;
		}

		Update();

		// check conditions
		if(State() == IClient::STATE_QUITING)
			break;

		// beNice
		if(Config()->m_ClCpuThrottle)
			thread_sleep(Config()->m_ClCpuThrottle);

		if(Config()->m_DbgHitch)
		{
			thread_sleep(Config()->m_DbgHitch);
			Config()->m_DbgHitch = 0;
		}

		/*
		if(ReportTime < time_get())
		{
			if(0 && Config()->m_Debug)
			{
				dbg_msg("client/report", "fps=%.02f (%.02f %.02f) netstate=%d",
					m_Frames/(float)(ReportInterval/time_freq()),
					1.0f/m_RenderFrameTimeHigh,
					1.0f/m_RenderFrameTimeLow,
					m_NetClient.State());
			}
			m_RenderFrameTimeLow = 1;
			m_RenderFrameTimeHigh = 0;
			m_RenderFrames = 0;
			ReportTime += ReportInterval;
		}*/

		// update local time
		m_LocalTime = (time_get()-m_LocalStartTime)/(float)time_freq();
	}

	GameClient()->OnShutdown();
	Disconnect();

	m_ServerBrowser.SaveServerlist();

	// shutdown SDL
	SDL_Quit();
}

int64 CClient::TickStartTime(int Tick)
{
	return m_LocalStartTime + (time_freq()*Tick)/m_GameTickSpeed;
}

void CClient::Con_Connect(IConsole::IResult *pResult, void *pUserData)
{
	CClient *pSelf = (CClient *)pUserData;
	str_copy(pSelf->m_aCmdConnect, pResult->GetString(0), sizeof(pSelf->m_aCmdConnect));
}

void CClient::Con_Disconnect(IConsole::IResult *pResult, void *pUserData)
{
	CClient *pSelf = (CClient *)pUserData;
	pSelf->Disconnect();
}

void CClient::Con_Quit(IConsole::IResult *pResult, void *pUserData)
{
	CClient *pSelf = (CClient *)pUserData;
	pSelf->Quit();
}

void CClient::Con_Minimize(IConsole::IResult *pResult, void *pUserData)
{
}

void CClient::Con_Ping(IConsole::IResult *pResult, void *pUserData)
{
	CClient *pSelf = (CClient *)pUserData;

	CMsgPacker Msg(NETMSG_PING, true);
	pSelf->SendMsg(&Msg, 0);
	pSelf->m_PingStartTime = time_get();
}

void CClient::Con_Rcon(IConsole::IResult *pResult, void *pUserData)
{
	CClient *pSelf = (CClient *)pUserData;
	pSelf->Rcon(pResult->GetString(0));
}

void CClient::ServerBrowserUpdate()
{
	m_ServerBrowser.RequestResort();
}

static CClient *CreateClient()
{
	CClient *pClient = static_cast<CClient *>(mem_alloc(sizeof(CClient)));
	mem_zero(pClient, sizeof(CClient));
	return new(pClient) CClient;
}

void CClient::ConnectOnStart(const char *pAddress)
{
	str_copy(m_aCmdConnect, pAddress, sizeof(m_aCmdConnect));
}

void CClient::DoVersionSpecificActions()
{
	Config()->m_ClLastVersionPlayed = CLIENT_VERSION;
}

/*
	Server Time
	Client Mirror Time
	Client Predicted Time

	Snapshot Latency
		Downstream latency

	Prediction Latency
		Upstream latency
*/
#if defined(CONF_PLATFORM_MACOSX)
extern "C" int TWMain(int argc, const char **argv) // ignore_convention
#else
int main(int argc, const char **argv) // ignore_convention
#endif
{
	cmdline_fix(&argc, &argv);
#if defined(CONF_FAMILY_WINDOWS)
	bool QuickEditMode = false;
	for(int i = 1; i < argc; i++) // ignore_convention
	{
		if(str_comp("--quickeditmode", argv[i]) == 0) // ignore_convention
		{
			QuickEditMode = true;
		}
	}
#endif

	bool UseDefaultConfig = false;
	for(int i = 1; i < argc; i++) // ignore_convention
	{
		if(str_comp("-d", argv[i]) == 0 || str_comp("--default", argv[i]) == 0) // ignore_convention
		{
			UseDefaultConfig = true;
			break;
		}
	}

	bool RandInitFailed = secure_random_init() != 0;

	CClient *pClient = CreateClient();
	IKernel *pKernel = IKernel::Create();
	pKernel->RegisterInterface(pClient);
	pClient->RegisterInterfaces();

	// create the components
	int FlagMask = CFGFLAG_CLIENT;
	IEngine *pEngine = CreateEngine("Teeworlds");
	IConsole *pConsole = CreateConsole(FlagMask);
	IStorage *pStorage = CreateStorage("Teeworlds", IStorage::STORAGETYPE_CLIENT, argc, argv); // ignore_convention
	IConfigManager *pConfigManager = CreateConfigManager();
	IEngineMap *pEngineMap = CreateEngineMap();
	IMapChecker *pMapChecker = CreateMapChecker();
	IEngineMasterServer *pEngineMasterServer = CreateEngineMasterServer();

	if(RandInitFailed)
	{
		dbg_msg("secure", "could not initialize secure RNG");
		return -1;
	}

	{
		bool RegisterFail = false;

		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pEngine);
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pConsole);
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pConfigManager);


		RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IEngineMap*>(pEngineMap)); // register as both
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IMap*>(pEngineMap));

		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pMapChecker);

		RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IEngineMasterServer*>(pEngineMasterServer)); // register as both
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IMasterServer*>(pEngineMasterServer));

		RegisterFail = RegisterFail || !pKernel->RegisterInterface(CreateGameClient());
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pStorage);

		if(RegisterFail)
			return -1;
	}

	pEngine->Init();
	pConfigManager->Init(FlagMask);
	pConsole->Init();
	pEngineMasterServer->Init();
	pEngineMasterServer->Load();

	// init client's interfaces
	pClient->InitInterfaces();

	pKernel->RequestInterface<IGameClient>()->OnConsoleInit();

	if(!UseDefaultConfig)
	{
		// execute config file
		if(!pConsole->ExecuteFile(SETTINGS_FILENAME ".cfg"))
			pConsole->ExecuteFile("settings.cfg"); // fallback to legacy naming scheme

		// execute autoexec file
		pConsole->ExecuteFile("autoexec.cfg");

		// parse the command line arguments
		if(argc > 1) // ignore_convention
		{
			const char *pAddress = 0;
			if(argc == 2)
			{
				pAddress = str_startswith(argv[1], "teeworlds:");
			}
			if(pAddress)
			{
				pClient->ConnectOnStart(pAddress);
			}
			else
			{
				pConsole->ParseArguments(argc - 1, &argv[1]);
			}
		}
	}

	pClient->DoVersionSpecificActions();

	// restore empty config strings to their defaults
	pConfigManager->RestoreStrings();

	pClient->Engine()->InitLogfile();

	// run the client
	dbg_msg("client", "starting...");
	pClient->Run();

	// wait for background jobs to finish
	pEngine->ShutdownJobs();

	// free components
	pClient->~CClient();
	mem_free(pClient);
	delete pKernel;
	delete pEngine;
	delete pConsole;
	delete pStorage;
	delete pConfigManager;
	delete pEngineMap;
	delete pMapChecker;
	delete pEngineMasterServer;

	secure_random_uninit();
	cmdline_free(argc, argv);
	return 0;
}
