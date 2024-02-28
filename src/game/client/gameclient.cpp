/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/engine.h>
#include <engine/map.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/version.h>

#include "gameclient.h"

#include "components/broadcast.h"
#include "components/chat.h"
#include "components/controls.h"
#include "components/emoticon.h"
#include "components/items.h"
#include "components/infomessages.h"
#include "components/motd.h"
#include "components/players.h"
#include "components/nameplates.h"
#include "components/spectator.h"
#include "components/voting.h"

inline void StrToInts(int *pInts, int Num, const char *pStr)
{
       int Index = 0;
       while(Num)
       {
               char aBuf[4] = {0,0,0,0};
               for(int c = 0; c < 4 && pStr[Index]; c++, Index++)
                       aBuf[c] = pStr[Index];
               *pInts = ((aBuf[0]+128)<<24)|((aBuf[1]+128)<<16)|((aBuf[2]+128)<<8)|(aBuf[3]+128);
               pInts++;
               Num--;
       }

       // null terminate
       pInts[-1] &= 0xffffff00;
}

inline void IntsToStr(const int *pInts, int Num, char *pStr)
{
       while(Num)
       {
               pStr[0] = (((*pInts)>>24)&0xff)-128;
               pStr[1] = (((*pInts)>>16)&0xff)-128;
               pStr[2] = (((*pInts)>>8)&0xff)-128;
               pStr[3] = ((*pInts)&0xff)-128;
               pStr += 4;
               pInts++;
               Num--;
       }

       // null terminate
       pStr[-1] = 0;
}

inline void AppendDecimals(char *pBuf, int Size, int Time, int Precision)
{
	if(Precision > 0)
	{
		char aInvalid[] = ".---";
		char aMSec[] = {
			'.',
			(char)('0' + (Time / 100) % 10),
			(char)('0' + (Time / 10) % 10),
			(char)('0' + Time % 10),
			0
		};
		char *pDecimals = Time < 0 ? aInvalid : aMSec;
		pDecimals[minimum(Precision, 3)+1] = 0;
		str_append(pBuf, pDecimals, Size);
	}
}

void FormatTime(char *pBuf, int Size, int Time, int Precision)
{
	if(Time < 0)
		str_copy(pBuf, "-:--", Size);
	else
		str_format(pBuf, Size, "%02d:%02d", Time / (60 * 1000), (Time / 1000) % 60);
	AppendDecimals(pBuf, Size, Time, Precision);
}

void FormatTimeDiff(char *pBuf, int Size, int Time, int Precision, bool ForceSign)
{
	const char *pPositive = ForceSign ? "+" : "";
	const char *pSign = Time < 0 ? "-" : pPositive;
	Time = absolute(Time);
	str_format(pBuf, Size, "%s%d", pSign, Time / 1000);
	AppendDecimals(pBuf, Size, Time, Precision);
}

// instantiate all systems
static CInfoMessages gs_InfoMessages;
static CChat gs_Chat;
static CMotd gs_Motd;
static CBroadcast gs_Broadcast;
static CControls gs_Controls;
static CEmoticon gs_Emoticon;
static CVoting gs_Voting;
static CSpectator gs_Spectator;

static CPlayers gs_Players;
static CNamePlates gs_NamePlates;
static CItems gs_Items;

CGameClient::CStack::CStack() { m_Num = 0; }
void CGameClient::CStack::Add(class CComponent *pComponent) { m_paComponents[m_Num++] = pComponent; }

const char *CGameClient::Version() const { return GAME_VERSION; }
const char *CGameClient::NetVersion() const { return GAME_NETVERSION; }
const char *CGameClient::NetVersionHashUsed() const { return GAME_NETVERSION_HASH_FORCED; }
const char *CGameClient::NetVersionHashReal() const{ return GAME_NETVERSION_HASH; }
int CGameClient::ClientVersion() const { return CLIENT_VERSION; }
const char *CGameClient::GetItemName(int Type) const { return m_NetObjHandler.GetObjName(Type); }

float CGameClient::GetAnimationPlaybackSpeed() const
{
	return 0.0f;
}

enum
{
	STR_TEAM_GAME,
	STR_TEAM_RED,
	STR_TEAM_BLUE,
	STR_TEAM_SPECTATORS,
};

static int GetStrTeam(int Team, bool Teamplay)
{
	if(Teamplay)
	{
		if(Team == TEAM_RED)
			return STR_TEAM_RED;
		else if(Team == TEAM_BLUE)
			return STR_TEAM_BLUE;
	}
	else if(Team == 0)
		return STR_TEAM_GAME;

	return STR_TEAM_SPECTATORS;
}

void CGameClient::GetPlayerLabel(char* aBuf, int BufferSize, int ClientID, const char* ClientName)
{
	// str_format(aBuf, BufferSize, "%2d:", ClientID);
	// str_format(aBuf, BufferSize, "%2d: %s", ClientID, ClientName);
	str_format(aBuf, BufferSize, "%s", ClientName);
}

enum
{
	DO_CHAT=0,
	DO_BROADCAST,
	DO_SPECIAL,

	PARA_NONE=0,
	PARA_I,
	PARA_II,
	PARA_III,
};

struct CGameMsg
{
	int m_Action;
	int m_ParaType;
	const char *m_pText;
};

static CGameMsg gs_GameMsgList[NUM_GAMEMSGS] = {
	{/*GAMEMSG_TEAM_SWAP*/ DO_CHAT, PARA_NONE, "Teams were swapped"}, // Localize("Teams were swapped")
	{/*GAMEMSG_SPEC_INVALIDID*/ DO_CHAT, PARA_NONE, "Invalid spectator id used"},   //!
	{/*GAMEMSG_TEAM_SHUFFLE*/ DO_CHAT, PARA_NONE, "Teams were shuffled"}, // Localize("Teams were shuffled")
	{/*GAMEMSG_TEAM_BALANCE*/ DO_CHAT, PARA_NONE, "Teams have been balanced"}, // Localize("Teams have been balanced")
	{/*GAMEMSG_CTF_DROP*/ DO_SPECIAL, PARA_NONE, ""},	// special - play ctf drop sound
	{/*GAMEMSG_CTF_RETURN*/ DO_SPECIAL, PARA_NONE, ""},	// special - play ctf return sound

	{/*GAMEMSG_TEAM_ALL*/ DO_SPECIAL, PARA_I, ""},	// special - add team name
	{/*GAMEMSG_TEAM_BALANCE_VICTIM*/ DO_SPECIAL, PARA_I, ""},	// special - add team name
	{/*GAMEMSG_CTF_GRAB*/ DO_SPECIAL, PARA_I, ""},	// special - play ctf grab sound based on team

	{/*GAMEMSG_CTF_CAPTURE*/ DO_SPECIAL, PARA_III, ""},	// special - play ctf capture sound + capture chat message

	{/*GAMEMSG_GAME_PAUSED*/ DO_SPECIAL, PARA_I, ""},	// special - add player name
};

void CGameClient::OnConsoleInit()
{
	dbg_msg("GAMECLIENT", "console init");
	m_InitComplete = false;
	m_pEngine = Kernel()->RequestInterface<IEngine>();
	m_pClient = Kernel()->RequestInterface<IClient>();
	m_pConfig = Kernel()->RequestInterface<IConfigManager>()->Values();
	m_pStorage = Kernel()->RequestInterface<IStorage>();

	// setup pointers
	m_pBroadcast = &::gs_Broadcast;
	m_pChat = &::gs_Chat;
	m_pControls = &::gs_Controls;
	m_pMotd = &::gs_Motd;
	m_pVoting = &::gs_Voting;
	m_pItems = &::gs_Items;

	// make a list of all the systems, make sure to add them in the corrent render order
	m_All.Add(m_pControls);
	m_All.Add(m_pVoting);

	m_All.Add(m_pItems);
	m_All.Add(&gs_Players);
	m_All.Add(&gs_NamePlates);
	m_All.Add(&gs_Spectator);
	m_All.Add(&gs_Emoticon);
	m_All.Add(&gs_InfoMessages);
	m_All.Add(m_pChat);
	m_All.Add(&gs_Broadcast);
	m_All.Add(m_pMotd);

	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->m_pClient = this;

	// let all the other components register their console commands
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnConsoleInit();

	//
	m_SuppressEvents = false;
}

void CGameClient::OnInit()
{
	int64 Start = time_get();

	// TODO: this should be different
	// setup item sizes
	// HACK: only set static size for items, which were available in the first 0.7 release
	// so new items don't break the snapshot delta
	static const int OLD_NUM_NETOBJTYPES = 23;
	for(int i = 0; i < OLD_NUM_NETOBJTYPES; i++)
		Client()->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));

	// determine total work for loading all components
	int TotalWorkAmount = 0;
	for(int i = m_All.m_Num-1; i >= 0; --i)
		TotalWorkAmount += m_All.m_paComponents[i]->GetInitAmount();

	// init all components
	for(int i = m_All.m_Num-1; i >= 0; --i)
		m_All.m_paComponents[i]->OnInit();

	OnReset();

	m_ServerMode = SERVERMODE_PURE;

	m_IsXmasDay = time_isxmasday();
	m_IsEasterDay = time_iseasterday();
	m_InitComplete = true;

	int64 End = time_get();
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "initialisation finished after %.2fms", ((End - Start) * 1000) / (float)time_freq());
	dbg_msg("gameclient", "%s", aBuf);
}

void CGameClient::OnUpdate()
{
}

int CGameClient::OnSnapInput(int *pData)
{
	return m_pControls->SnapInput(pData);
}

void CGameClient::OnConnected()
{
	for(int i = 0; i < m_All.m_Num; i++)
	{
		m_All.m_paComponents[i]->OnMapLoad();
		m_All.m_paComponents[i]->OnReset();
	}

	m_ServerMode = SERVERMODE_PURE;

	// send the inital info
	SendStartInfo();
}

void CGameClient::OnReset()
{
	if(Client()->State() < IClient::STATE_ONLINE)
	{
		// clear out the invalid pointers
		m_LastNewPredictedTick = -1;
		mem_zero(&m_Snap, sizeof(m_Snap));

		for(int ClientID = 0; ClientID < MAX_CLIENTS; ClientID++)
			m_aClients[ClientID].Reset(this, ClientID);
	}

	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnReset();

	if(Client()->State() < IClient::STATE_ONLINE)
	{
		m_LocalClientID = -1;
		m_TeamCooldownTick = 0;
		m_TeamChangeTime = 0.0f;
		m_LastSkinChangeTime = Client()->LocalTime();
		m_IdentityState = -1;
		mem_zero(&m_GameInfo, sizeof(m_GameInfo));
		m_DemoSpecMode = SPEC_FREEVIEW;
		m_DemoSpecID = -1;
		m_LastGameStartTick = -1;
		m_LastFlagCarrierRed = FLAG_MISSING;
		m_LastFlagCarrierBlue = FLAG_MISSING;
	}
}

void CGameClient::UpdatePositions()
{
	// `m_LocalCharacterPos` is used for many things besides rendering the
	// player (e.g. camera position, mouse input), which is why we set it here.
	if(m_Snap.m_pLocalCharacter && m_Snap.m_pLocalPrevCharacter)
	{
		m_LocalCharacterPos = GetCharPos(m_LocalClientID, false);
	}

	// spectator position
	if(m_Snap.m_SpecInfo.m_Active)
	{
		if(
			m_Snap.m_pSpectatorInfo &&
			(
				Client()->State() == IClient::STATE_DEMOPLAYBACK ||
				m_Snap.m_SpecInfo.m_SpecMode != SPEC_FREEVIEW ||
				(
					m_Snap.m_pLocalInfo &&
					(m_Snap.m_pLocalInfo->m_PlayerFlags & PLAYERFLAG_DEAD) &&
					m_Snap.m_SpecInfo.m_SpecMode != SPEC_FREEVIEW
				)
			))
		{
			if(m_Snap.m_pPrevSpectatorInfo)
				m_Snap.m_SpecInfo.m_Position = mix(
					vec2(
						m_Snap.m_pPrevSpectatorInfo->m_X,
						m_Snap.m_pPrevSpectatorInfo->m_Y
					),
					vec2(
						m_Snap.m_pSpectatorInfo->m_X,
						m_Snap.m_pSpectatorInfo->m_Y
					),
					Client()->IntraGameTick()
				);
			else
				m_Snap.m_SpecInfo.m_Position = vec2(
					m_Snap.m_pSpectatorInfo->m_X,
					m_Snap.m_pSpectatorInfo->m_Y
				);

			m_LocalCharacterPos = m_Snap.m_SpecInfo.m_Position;
			m_Snap.m_SpecInfo.m_UsePosition = true;
		}
	}
}

void CGameClient::StartRendering()
{
}

void CGameClient::OnRender()
{
	// update the local character and spectate position
	UpdatePositions();

	StartRendering();

	// render all systems
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnRender();

}

void CGameClient::OnRelease()
{
	// release all systems
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnRelease();
}

void CGameClient::OnMessage(int MsgId, CUnpacker *pUnpacker)
{
	// special messages
	if(MsgId == NETMSGTYPE_SV_TUNEPARAMS && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
	}
	else if(MsgId == NETMSGTYPE_SV_VOTEOPTIONLISTADD)
	{
		int NumOptions = pUnpacker->GetInt();
		for(int i = 0; i < NumOptions; i++)
		{
			const char *pDescription = pUnpacker->GetString(CUnpacker::SANITIZE_CC);
			if(pUnpacker->Error())
				return;

			m_pVoting->AddOption(pDescription);
		}
	}
	else if(MsgId == NETMSGTYPE_SV_GAMEMSG)
	{
		int GameMsgID = pUnpacker->GetInt();

		// check for valid gamemsgid
		if(GameMsgID < 0 || GameMsgID >= NUM_GAMEMSGS)
			return;

		int aParaI[3];
		int NumParaI = 0;

		// get paras
		switch(gs_GameMsgList[GameMsgID].m_ParaType)
		{
		case PARA_I: NumParaI = 1; break;
		case PARA_II: NumParaI = 2; break;
		case PARA_III: NumParaI = 3; break;
		}
		for(int i = 0; i < NumParaI; i++)
		{
			aParaI[i] = pUnpacker->GetInt();
		}

		// check for unpacking errors
		if(pUnpacker->Error())
			return;

		// handle special messages
		char aBuf[256];
		bool TeamPlay = m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS;
		if(gs_GameMsgList[GameMsgID].m_Action == DO_SPECIAL)
		{
			switch(GameMsgID)
			{
			case GAMEMSG_CTF_DROP:
				dbg_msg("sound", "CSounds::CHN_GLOBAL, SOUND_CTF_DROP);");
				break;
			case GAMEMSG_CTF_RETURN:
				dbg_msg("sound", "CSounds::CHN_GLOBAL, SOUND_CTF_RETURN);");
				break;
			case GAMEMSG_TEAM_ALL:
				{
					const char *pMsg = "";
					switch(GetStrTeam(aParaI[0], TeamPlay))
					{
					case STR_TEAM_GAME: pMsg = Localize("All players were moved to the game"); break;
					case STR_TEAM_RED: pMsg = Localize("All players were moved to the red team"); break;
					case STR_TEAM_BLUE: pMsg = Localize("All players were moved to the blue team"); break;
					case STR_TEAM_SPECTATORS: pMsg = Localize("All players were moved to the spectators"); break;
					}
					m_pBroadcast->DoClientBroadcast(pMsg);
				}
				break;
			case GAMEMSG_TEAM_BALANCE_VICTIM:
				{
					const char *pMsg = "";
					switch(GetStrTeam(aParaI[0], TeamPlay))
					{
					case STR_TEAM_RED: pMsg = Localize("You were moved to the red team due to team balancing"); break;
					case STR_TEAM_BLUE: pMsg = Localize("You were moved to the blue team due to team balancing"); break;
					}
					m_pBroadcast->DoClientBroadcast(pMsg);
				}
				break;
			case GAMEMSG_CTF_GRAB:
				if(m_LocalClientID != -1 && (m_aClients[m_LocalClientID].m_Team != aParaI[0] || (m_Snap.m_SpecInfo.m_Active &&
								((m_Snap.m_SpecInfo.m_SpectatorID != -1 && m_aClients[m_Snap.m_SpecInfo.m_SpectatorID].m_Team != aParaI[0]) ||
								(m_Snap.m_SpecInfo.m_SpecMode == SPEC_FLAGRED && aParaI[0] != TEAM_RED) ||
								(m_Snap.m_SpecInfo.m_SpecMode == SPEC_FLAGBLUE && aParaI[0] != TEAM_BLUE)))))
					dbg_msg("sound", "CSounds::CHN_GLOBAL, SOUND_CTF_GRAB_PL);");
				else
					dbg_msg("sound", "CSounds::CHN_GLOBAL, SOUND_CTF_GRAB_EN);");
				break;
			case GAMEMSG_GAME_PAUSED:
				{
					int ClientID = clamp(aParaI[0], 0, MAX_CLIENTS - 1);
					char aLabel[64];
					GetPlayerLabel(aLabel, sizeof(aLabel), ClientID, m_aClients[ClientID].m_aName);
					str_format(aBuf, sizeof(aBuf), Localize("'%s' initiated a pause"), aLabel);
					m_pChat->AddLine(aBuf);
				}
				break;
			case GAMEMSG_CTF_CAPTURE:
				dbg_msg("sound", "CSounds::CHN_GLOBAL, SOUND_CTF_CAPTURE);");
				int ClientID = clamp(aParaI[1], 0, MAX_CLIENTS - 1);
				// m_pStats->OnFlagCapture(ClientID);
				char aLabel[64];
				GetPlayerLabel(aLabel, sizeof(aLabel), ClientID, m_aClients[ClientID].m_aName);

				float Time = aParaI[2] / (float)Client()->GameTickSpeed();
				if(Time <= 60)
				{
					if(aParaI[0])
					{
						str_format(aBuf, sizeof(aBuf), Localize("The blue flag was captured by '%s' (%.2f seconds)"), aLabel, Time);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), Localize("The red flag was captured by '%s' (%.2f seconds)"), aLabel, Time);
					}
				}
				else
				{
					if(aParaI[0])
					{
						str_format(aBuf, sizeof(aBuf), Localize("The blue flag was captured by '%s'"), aLabel);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), Localize("The red flag was captured by '%s'"), aLabel);
					}
				}
				m_pChat->AddLine(aBuf);
			}
			return;
		}

		// build message
		const char *pText = "";
		if(NumParaI == 0)
		{
			pText = Localize(gs_GameMsgList[GameMsgID].m_pText);
		}

		// handle message
		switch(gs_GameMsgList[GameMsgID].m_Action)
		{
		case DO_CHAT:
			m_pChat->AddLine(pText);
			break;
		case DO_BROADCAST:
			m_pBroadcast->DoClientBroadcast(pText);
			break;
		}
	}

	void *pRawMsg = m_NetObjHandler.SecureUnpackMsg(MsgId, pUnpacker);
	if(!pRawMsg)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "dropped weird message '%s' (%d), failed on '%s'", m_NetObjHandler.GetMsgName(MsgId), MsgId, m_NetObjHandler.FailedMsgOn());
		dbg_msg("client", "%s", aBuf);
		return;
	}

	// TODO: this should be done smarter
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnMessage(MsgId, pRawMsg);

	if(MsgId == NETMSGTYPE_SV_CLIENTINFO && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		CNetMsg_Sv_ClientInfo *pMsg = (CNetMsg_Sv_ClientInfo *)pRawMsg;

		if(pMsg->m_Local)
		{
			if(m_LocalClientID != -1)
			{
				if(Config()->m_Debug)
					dbg_msg("client", "invalid local clientinfo");
				return;
			}
			m_LocalClientID = pMsg->m_ClientID;
			m_TeamChangeTime = Client()->LocalTime();
		}
		else
		{
			if(m_aClients[pMsg->m_ClientID].m_Active)
			{
				if(Config()->m_Debug)
					dbg_msg("client", "invalid clientinfo");
				return;
			}

			if(m_LocalClientID != -1 && !pMsg->m_Silent)
			{
				DoEnterMessage(pMsg->m_pName, pMsg->m_ClientID, pMsg->m_Team);
			}
		}

		m_aClients[pMsg->m_ClientID].m_Active = true;
		m_aClients[pMsg->m_ClientID].m_Team  = pMsg->m_Team;
		str_utf8_copy_num(m_aClients[pMsg->m_ClientID].m_aName, pMsg->m_pName, sizeof(m_aClients[pMsg->m_ClientID].m_aName), MAX_NAME_LENGTH);
		str_utf8_copy_num(m_aClients[pMsg->m_ClientID].m_aClan, pMsg->m_pClan, sizeof(m_aClients[pMsg->m_ClientID].m_aClan), MAX_CLAN_LENGTH);
		m_aClients[pMsg->m_ClientID].m_Country = pMsg->m_Country;
		for(int i = 0; i < NUM_SKINPARTS; i++)
		{
			str_utf8_copy_num(m_aClients[pMsg->m_ClientID].m_aaSkinPartNames[i], pMsg->m_apSkinPartNames[i], sizeof(m_aClients[pMsg->m_ClientID].m_aaSkinPartNames[i]), MAX_SKIN_LENGTH);
			m_aClients[pMsg->m_ClientID].m_aUseCustomColors[i] = pMsg->m_aUseCustomColors[i];
			m_aClients[pMsg->m_ClientID].m_aSkinPartColors[i] = pMsg->m_aSkinPartColors[i];
		}

		// update friend state
		m_aClients[pMsg->m_ClientID].m_Friend = false;
		// update chat ignore state
		m_aClients[pMsg->m_ClientID].m_ChatIgnore = false;
		if(m_aClients[pMsg->m_ClientID].m_ChatIgnore)
		{
			char aBuf[128];
			char aLabel[64];
			GetPlayerLabel(aLabel, sizeof(aLabel), pMsg->m_ClientID, m_aClients[pMsg->m_ClientID].m_aName);
			str_format(aBuf, sizeof(aBuf), Localize("%s is muted by you"), aLabel);
			m_pChat->AddLine(aBuf, CChat::CLIENT_MSG);
		}

		m_aClients[pMsg->m_ClientID].UpdateRenderInfo(this, pMsg->m_ClientID, true);

		m_GameInfo.m_NumPlayers++;
		// calculate team-balance
		if(m_aClients[pMsg->m_ClientID].m_Team != TEAM_SPECTATORS)
			m_GameInfo.m_aTeamSize[m_aClients[pMsg->m_ClientID].m_Team]++;

		// m_pStats->OnPlayerEnter(pMsg->m_ClientID, pMsg->m_Team);
	}
	else if(MsgId == NETMSGTYPE_SV_CLIENTDROP && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		CNetMsg_Sv_ClientDrop *pMsg = (CNetMsg_Sv_ClientDrop *)pRawMsg;

		if(m_LocalClientID == pMsg->m_ClientID || !m_aClients[pMsg->m_ClientID].m_Active)
		{
			if(Config()->m_Debug)
				dbg_msg("client", "invalid clientdrop");
			return;
		}

		if(!pMsg->m_Silent)
		{
			DoLeaveMessage(m_aClients[pMsg->m_ClientID].m_aName, pMsg->m_ClientID, pMsg->m_pReason);

			CNetMsg_De_ClientLeave Msg;
			Msg.m_pName = m_aClients[pMsg->m_ClientID].m_aName;
			Msg.m_ClientID = pMsg->m_ClientID;
			Msg.m_pReason = pMsg->m_pReason;
			Client()->SendPackMsg(&Msg, MSGFLAG_NOSEND | MSGFLAG_RECORD);
		}

		m_GameInfo.m_NumPlayers--;
		// calculate team-balance
		if(m_aClients[pMsg->m_ClientID].m_Team != TEAM_SPECTATORS)
			m_GameInfo.m_aTeamSize[m_aClients[pMsg->m_ClientID].m_Team]--;

		m_aClients[pMsg->m_ClientID].Reset(this, pMsg->m_ClientID);
		// m_pStats->OnPlayerLeave(pMsg->m_ClientID);
	}
	else if(MsgId == NETMSGTYPE_SV_SKINCHANGE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		CNetMsg_Sv_SkinChange *pMsg = (CNetMsg_Sv_SkinChange *)pRawMsg;

		if(!m_aClients[pMsg->m_ClientID].m_Active)
		{
			if(Config()->m_Debug)
				dbg_msg("client", "invalid skin info");
			return;
		}

		for(int i = 0; i < NUM_SKINPARTS; i++)
		{
			str_utf8_copy_num(m_aClients[pMsg->m_ClientID].m_aaSkinPartNames[i], pMsg->m_apSkinPartNames[i], sizeof(m_aClients[pMsg->m_ClientID].m_aaSkinPartNames[i]), MAX_SKIN_LENGTH);
			m_aClients[pMsg->m_ClientID].m_aUseCustomColors[i] = pMsg->m_aUseCustomColors[i];
			m_aClients[pMsg->m_ClientID].m_aSkinPartColors[i] = pMsg->m_aSkinPartColors[i];
		}
		m_aClients[pMsg->m_ClientID].UpdateRenderInfo(this, pMsg->m_ClientID, true);
	}
	else if(MsgId == NETMSGTYPE_SV_GAMEINFO && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		CNetMsg_Sv_GameInfo *pMsg = (CNetMsg_Sv_GameInfo *)pRawMsg;

		m_GameInfo.m_GameFlags = pMsg->m_GameFlags;
		m_GameInfo.m_ScoreLimit = pMsg->m_ScoreLimit;
		m_GameInfo.m_TimeLimit = pMsg->m_TimeLimit;
		m_GameInfo.m_MatchNum = pMsg->m_MatchNum;
		m_GameInfo.m_MatchCurrent = pMsg->m_MatchCurrent;
	}
	else if(MsgId == NETMSGTYPE_SV_SERVERSETTINGS && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		CNetMsg_Sv_ServerSettings *pMsg = (CNetMsg_Sv_ServerSettings *)pRawMsg;

		if(!m_ServerSettings.m_TeamLock && pMsg->m_TeamLock)
			m_pChat->AddLine(Localize("Teams were locked"));
		else if(m_ServerSettings.m_TeamLock && !pMsg->m_TeamLock)
			m_pChat->AddLine(Localize("Teams were unlocked"));

		m_ServerSettings.m_KickVote = pMsg->m_KickVote;
		m_ServerSettings.m_KickMin = pMsg->m_KickMin;
		m_ServerSettings.m_SpecVote = pMsg->m_SpecVote;
		m_ServerSettings.m_TeamLock = pMsg->m_TeamLock;
		m_ServerSettings.m_TeamBalance = pMsg->m_TeamBalance;
		m_ServerSettings.m_PlayerSlots = pMsg->m_PlayerSlots;
		dbg_msg(
			"network_in",
			"NETMSGTYPE_SV_SERVERSETTINGS playerslots=%d specvote=%d",
			pMsg->m_PlayerSlots,
			pMsg->m_SpecVote
		);
	}
	else if(MsgId == NETMSGTYPE_SV_TEAM)
	{
		CNetMsg_Sv_Team *pMsg = (CNetMsg_Sv_Team *)pRawMsg;

		if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
		{
			// calculate team-balance
			if(m_aClients[pMsg->m_ClientID].m_Team != TEAM_SPECTATORS)
				m_GameInfo.m_aTeamSize[m_aClients[pMsg->m_ClientID].m_Team]--;
			m_aClients[pMsg->m_ClientID].m_Team = pMsg->m_Team;
			if(m_aClients[pMsg->m_ClientID].m_Team != TEAM_SPECTATORS)
				m_GameInfo.m_aTeamSize[m_aClients[pMsg->m_ClientID].m_Team]++;

			m_aClients[pMsg->m_ClientID].UpdateRenderInfo(this, pMsg->m_ClientID, false);

			if(pMsg->m_ClientID == m_LocalClientID)
			{
				m_TeamCooldownTick = pMsg->m_CooldownTick;
				m_TeamChangeTime = Client()->LocalTime();
			}
		}

		if(pMsg->m_Silent == 0)
		{
			DoTeamChangeMessage(m_aClients[pMsg->m_ClientID].m_aName, pMsg->m_ClientID, pMsg->m_Team);
		}
	}
	else if(MsgId == NETMSGTYPE_SV_READYTOENTER)
	{
		Client()->EnterGame();
	}
	else if (MsgId == NETMSGTYPE_SV_EMOTICON)
	{
		CNetMsg_Sv_Emoticon *pMsg = (CNetMsg_Sv_Emoticon *)pRawMsg;

		// apply
		m_aClients[pMsg->m_ClientID].m_Emoticon = pMsg->m_Emoticon;
		m_aClients[pMsg->m_ClientID].m_EmoticonStart = Client()->GameTick();
	}
	else if(MsgId == NETMSGTYPE_DE_CLIENTENTER && Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		CNetMsg_De_ClientEnter *pMsg = (CNetMsg_De_ClientEnter *)pRawMsg;
		DoEnterMessage(pMsg->m_pName, pMsg->m_ClientID, pMsg->m_Team);
		// m_pStats->OnPlayerEnter(pMsg->m_ClientID, pMsg->m_Team);
	}
	else if(MsgId == NETMSGTYPE_DE_CLIENTLEAVE && Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		CNetMsg_De_ClientLeave *pMsg = (CNetMsg_De_ClientLeave *)pRawMsg;
		DoLeaveMessage(pMsg->m_pName, pMsg->m_ClientID, pMsg->m_pReason);
		// m_pStats->OnPlayerLeave(pMsg->m_ClientID);
	}
}

void CGameClient::OnStateChange(int NewState, int OldState)
{
	// reset everything when not already connected (to keep gathered stuff)
	if(NewState < IClient::STATE_ONLINE)
		OnReset();

	// then change the state
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnStateChange(NewState, OldState);
}

void CGameClient::OnShutdown()
{
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnShutdown();
}
void CGameClient::OnEnterGame() {}

void CGameClient::OnGameOver()
{
}

void CGameClient::OnStartGame()
{
}

void CGameClient::OnRconLine(const char *pLine)
{
	dbg_msg("rcon", "%s", pLine);
}

void CGameClient::ProcessEvents()
{
	if(m_SuppressEvents)
		return;

	int SnapType = IClient::SNAP_CURRENT;
	int Num = Client()->SnapNumItems(SnapType);
	for(int Index = 0; Index < Num; Index++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(SnapType, Index, &Item);

		if(Item.m_Type == NETEVENTTYPE_DAMAGE)
		{
			// CNetEvent_Damage *ev = (CNetEvent_Damage *)pData;
			// m_pEffects->DamageIndicator(vec2(ev->m_X, ev->m_Y), ev->m_HealthAmount + ev->m_ArmorAmount, ev->m_Angle / 256.0f, ev->m_ClientID);
		}
		else if(Item.m_Type == NETEVENTTYPE_EXPLOSION)
		{
			// CNetEvent_Explosion *ev = (CNetEvent_Explosion *)pData;
			// m_pEffects->Explosion(vec2(ev->m_X, ev->m_Y));
		}
		else if(Item.m_Type == NETEVENTTYPE_HAMMERHIT)
		{
			// CNetEvent_HammerHit *ev = (CNetEvent_HammerHit *)pData;
			// m_pEffects->HammerHit(vec2(ev->m_X, ev->m_Y));
		}
		else if(Item.m_Type == NETEVENTTYPE_SPAWN)
		{
			// CNetEvent_Spawn *ev = (CNetEvent_Spawn *)pData;
			// m_pEffects->PlayerSpawn(vec2(ev->m_X, ev->m_Y));
		}
		else if(Item.m_Type == NETEVENTTYPE_DEATH)
		{
			// CNetEvent_Death *ev = (CNetEvent_Death *)pData;
			// m_pEffects->PlayerDeath(vec2(ev->m_X, ev->m_Y), ev->m_ClientID);
		}
		else if(Item.m_Type == NETEVENTTYPE_SOUNDWORLD)
		{
			CNetEvent_SoundWorld *ev = (CNetEvent_SoundWorld *)pData;
			dbg_msg("sound", "(CSounds::CHN_WORLD, ev->m_SoundID=%d, 1.0f, vec2(ev->m_X, ev->m_Y));", ev->m_SoundID);
		}
	}
}

void CGameClient::ProcessTriggeredEvents(int Events, vec2 Pos)
{
	if(m_SuppressEvents)
		return;

	if(Events&COREEVENTFLAG_GROUND_JUMP)
		dbg_msg("sound", "(CSounds::CHN_WORLD, SOUND_PLAYER_JUMP, 1.0f, Pos);");
	// if(Events&COREEVENTFLAG_AIR_JUMP)
	// 	m_pEffects->AirJump(Pos);
	if(Events&COREEVENTFLAG_HOOK_ATTACH_PLAYER)
		dbg_msg("sound", "(CSounds::CHN_WORLD, SOUND_HOOK_ATTACH_PLAYER, 1.0f, Pos);");
	if(Events&COREEVENTFLAG_HOOK_ATTACH_GROUND)
		dbg_msg("sound", "(CSounds::CHN_WORLD, SOUND_HOOK_ATTACH_GROUND, 1.0f, Pos);");
	if(Events&COREEVENTFLAG_HOOK_HIT_NOHOOK)
		dbg_msg("sound", "(CSounds::CHN_WORLD, SOUND_HOOK_NOATTACH, 1.0f, Pos);");
	/*if(Events&COREEVENTFLAG_HOOK_LAUNCH)
		dbg_msg("sound", "(CSounds::CHN_WORLD, SOUND_HOOK_LOOP, 1.0f, Pos);");
	if(Events&COREEVENTFLAG_HOOK_RETRACT)
		dbg_msg("sound", "(CSounds::CHN_WORLD, SOUND_PLAYER_JUMP, 1.0f, Pos);*/
}

typedef bool (*FCompareFunc)(const CNetObj_PlayerInfo*, const CNetObj_PlayerInfo*);

bool CompareScore(const CNetObj_PlayerInfo *Pl1, const CNetObj_PlayerInfo *Pl2)
{
	return Pl1->m_Score < Pl2->m_Score;
}

bool CompareTime(const CNetObj_PlayerInfo *Pl1, const CNetObj_PlayerInfo *Pl2)
{
	if(Pl1->m_Score < 0)
		return true;
	if(Pl2->m_Score < 0)
		return false;
	return Pl1->m_Score > Pl2->m_Score;
}

void CGameClient::OnNewSnapshot()
{
	// clear out the invalid pointers
	mem_zero(&m_Snap, sizeof(m_Snap));

	// secure snapshot
	{
		int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
		for(int Index = 0; Index < Num; Index++)
		{
			IClient::CSnapItem Item;
			const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, Index, &Item);
			if(m_NetObjHandler.ValidateObj(Item.m_Type, pData, Item.m_DataSize) != 0)
			{
				if(Config()->m_Debug)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "invalidated index=%d type=%d (%s) size=%d id=%d", Index, Item.m_Type, m_NetObjHandler.GetObjName(Item.m_Type), Item.m_DataSize, Item.m_ID);
					dbg_msg("game", "%s", aBuf);
				}
				Client()->SnapInvalidateItem(IClient::SNAP_CURRENT, Index);
			}
		}
	}

	ProcessEvents();

	// go trough all the items in the snapshot and gather the info we want
	{
		int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
		for(int i = 0; i < Num; i++)
		{
			IClient::CSnapItem Item;
			const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

			// demo items
			if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
			{
				if(Item.m_Type == NETOBJTYPE_DE_CLIENTINFO)
				{
					const CNetObj_De_ClientInfo *pInfo = (const CNetObj_De_ClientInfo *)pData;
					int ClientID = Item.m_ID;
					if(ClientID < MAX_CLIENTS)
					{
						CClientData *pClient = &m_aClients[ClientID];

						if(pInfo->m_Local)
							m_LocalClientID = ClientID;
						pClient->m_Active = true;
						pClient->m_Team  = pInfo->m_Team;
						IntsToStr(pInfo->m_aName, 4, pClient->m_aName);
						IntsToStr(pInfo->m_aClan, 3, pClient->m_aClan);
						pClient->m_Country = pInfo->m_Country;

						for(int p = 0; p < NUM_SKINPARTS; p++)
						{
							IntsToStr(pInfo->m_aaSkinPartNames[p], 6, pClient->m_aaSkinPartNames[p]);
							pClient->m_aUseCustomColors[p] = pInfo->m_aUseCustomColors[p];
							pClient->m_aSkinPartColors[p] = pInfo->m_aSkinPartColors[p];
						}

						m_GameInfo.m_NumPlayers++;
						// calculate team-balance
						if(pClient->m_Team != TEAM_SPECTATORS)
							m_GameInfo.m_aTeamSize[pClient->m_Team]++;
					}
				}
				else if(Item.m_Type == NETOBJTYPE_DE_GAMEINFO)
				{
					const CNetObj_De_GameInfo *pInfo = (const CNetObj_De_GameInfo *)pData;

					m_GameInfo.m_GameFlags = pInfo->m_GameFlags;
					m_GameInfo.m_ScoreLimit = pInfo->m_ScoreLimit;
					m_GameInfo.m_TimeLimit = pInfo->m_TimeLimit;
					m_GameInfo.m_MatchNum = pInfo->m_MatchNum;
					m_GameInfo.m_MatchCurrent = pInfo->m_MatchCurrent;
				}
				else if(Item.m_Type == NETOBJTYPE_DE_TUNEPARAMS)
				{
					// const CNetObj_De_TuneParams *pInfo = (const CNetObj_De_TuneParams *)pData;
				}
			}

			// network items
			if(Item.m_Type == NETOBJTYPE_PLAYERINFO)
			{
				const CNetObj_PlayerInfo *pInfo = (const CNetObj_PlayerInfo *)pData;
				int ClientID = Item.m_ID;
				if(ClientID < MAX_CLIENTS && m_aClients[ClientID].m_Active)
				{
					m_Snap.m_paPlayerInfos[ClientID] = pInfo;
					m_Snap.m_aInfoByScore[ClientID].m_pPlayerInfo = pInfo;
					m_Snap.m_aInfoByScore[ClientID].m_ClientID = ClientID;

					if(m_LocalClientID == ClientID)
					{
						m_Snap.m_pLocalInfo = pInfo;

						if(m_aClients[ClientID].m_Team == TEAM_SPECTATORS)
						{
							m_Snap.m_SpecInfo.m_Active = true;
							m_Snap.m_SpecInfo.m_SpecMode = SPEC_FREEVIEW;
							m_Snap.m_SpecInfo.m_SpectatorID = -1;
						}
					}
				}
			}
			else if(Item.m_Type == NETOBJTYPE_PLAYERINFORACE)
			{
				const CNetObj_PlayerInfoRace *pInfo = (const CNetObj_PlayerInfoRace *)pData;
				int ClientID = Item.m_ID;
				if(ClientID < MAX_CLIENTS && m_aClients[ClientID].m_Active)
				{
					m_Snap.m_paPlayerInfosRace[ClientID] = pInfo;
				}
			}
			else if(Item.m_Type == NETOBJTYPE_CHARACTER)
			{
				if(Item.m_ID < MAX_CLIENTS)
				{
					CSnapState::CCharacterInfo *pCharInfo = &m_Snap.m_aCharacters[Item.m_ID];
					const void *pOld = Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_CHARACTER, Item.m_ID);
					pCharInfo->m_Cur = *((const CNetObj_Character *)pData);

					// clamp ammo count for non ninja weapon
					if(pCharInfo->m_Cur.m_Weapon != WEAPON_NINJA)
						pCharInfo->m_Cur.m_AmmoCount = clamp(pCharInfo->m_Cur.m_AmmoCount, 0, 10);

					if(pOld)
					{
						pCharInfo->m_Active = true;
						pCharInfo->m_Prev = *((const CNetObj_Character *)pOld);

						// limit evolving to 3 seconds
						int EvolvePrevTick = minimum(pCharInfo->m_Prev.m_Tick + Client()->GameTickSpeed()*3, Client()->PrevGameTick());
						// int EvolveCurTick = minimum(pCharInfo->m_Cur.m_Tick + Client()->GameTickSpeed()*3, Client()->GameTick());

						// reuse the evolved char
						if(m_aClients[Item.m_ID].m_Evolved.m_Tick == EvolvePrevTick)
						{
							pCharInfo->m_Prev = m_aClients[Item.m_ID].m_Evolved;
							if(mem_comp(pData, pOld, sizeof(CNetObj_Character)) == 0)
								pCharInfo->m_Cur = m_aClients[Item.m_ID].m_Evolved;
						}

						m_aClients[Item.m_ID].m_Evolved = m_Snap.m_aCharacters[Item.m_ID].m_Cur;
					}

					if(Item.m_ID != m_LocalClientID || Client()->State() == IClient::STATE_DEMOPLAYBACK)
						ProcessTriggeredEvents(pCharInfo->m_Cur.m_TriggeredEvents, vec2(pCharInfo->m_Cur.m_X, pCharInfo->m_Cur.m_Y));
				}
			}
			else if(Item.m_Type == NETOBJTYPE_SPECTATORINFO)
			{
				m_Snap.m_pSpectatorInfo = (const CNetObj_SpectatorInfo *)pData;
				m_Snap.m_pPrevSpectatorInfo = (const CNetObj_SpectatorInfo *)Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_SPECTATORINFO, Item.m_ID);
				m_Snap.m_SpecInfo.m_Active = true;
				m_Snap.m_SpecInfo.m_SpecMode = m_Snap.m_pSpectatorInfo->m_SpecMode;
				m_Snap.m_SpecInfo.m_SpectatorID = m_Snap.m_pSpectatorInfo->m_SpectatorID;
			}
			else if(Item.m_Type == NETOBJTYPE_GAMEDATA)
			{
				m_Snap.m_pGameData = (const CNetObj_GameData *)pData;

				static int s_LastGameFlags = 0;
				int GameFlags = m_Snap.m_pGameData->m_GameStateFlags;
				if(!(s_LastGameFlags&GAMESTATEFLAG_GAMEOVER) && GameFlags&GAMESTATEFLAG_GAMEOVER)
					OnGameOver();
				else if(s_LastGameFlags&GAMESTATEFLAG_GAMEOVER && !(GameFlags&GAMESTATEFLAG_GAMEOVER))
					OnStartGame();

				if(m_Snap.m_pGameData->m_GameStartTick != m_LastGameStartTick && !(s_LastGameFlags&GAMESTATEFLAG_ROUNDOVER)
					&& !(s_LastGameFlags&GAMESTATEFLAG_PAUSED) && (!(GameFlags&GAMESTATEFLAG_PAUSED) || GameFlags&GAMESTATEFLAG_STARTCOUNTDOWN))
				{
					// sts OnMatchStart();
				}

				// if(!(GameFlags&(GAMESTATEFLAG_PAUSED|GAMESTATEFLAG_ROUNDOVER|GAMESTATEFLAG_GAMEOVER)))
					// m_pStats->UpdatePlayTime(Client()->GameTick() - Client()->PrevGameTick());

				s_LastGameFlags = GameFlags;
				m_LastGameStartTick = m_Snap.m_pGameData->m_GameStartTick;
			}
			else if(Item.m_Type == NETOBJTYPE_GAMEDATATEAM)
			{
				m_Snap.m_pGameDataTeam = (const CNetObj_GameDataTeam *)pData;
			}
			else if(Item.m_Type == NETOBJTYPE_GAMEDATAFLAG)
			{
				m_Snap.m_pGameDataFlag = (const CNetObj_GameDataFlag *)pData;
				m_Snap.m_GameDataFlagSnapID = Item.m_ID;

				// // stats
				// if(m_LastFlagCarrierRed == FLAG_ATSTAND && m_Snap.m_pGameDataFlag->m_FlagCarrierRed >= 0)
				// 	m_pStats->OnFlagGrab(m_Snap.m_pGameDataFlag->m_FlagCarrierRed);
				// else if(m_LastFlagCarrierBlue == FLAG_ATSTAND && m_Snap.m_pGameDataFlag->m_FlagCarrierBlue >= 0)
				// 	m_pStats->OnFlagGrab(m_Snap.m_pGameDataFlag->m_FlagCarrierBlue);

				m_LastFlagCarrierRed = m_Snap.m_pGameDataFlag->m_FlagCarrierRed;
				m_LastFlagCarrierBlue = m_Snap.m_pGameDataFlag->m_FlagCarrierBlue;
			}
			else if(Item.m_Type == NETOBJTYPE_GAMEDATARACE)
			{
				m_Snap.m_pGameDataRace = (const CNetObj_GameDataRace *)pData;
			}
			else if(Item.m_Type == NETOBJTYPE_FLAG)
			{
				m_Snap.m_paFlags[Item.m_ID%2] = (const CNetObj_Flag *)pData;
			}
		}
	}

	// setup local pointers
	if(m_LocalClientID >= 0)
	{
		CSnapState::CCharacterInfo *c = &m_Snap.m_aCharacters[m_LocalClientID];
		if(c->m_Active)
		{
			if (!m_Snap.m_SpecInfo.m_Active)
			{
				m_Snap.m_pLocalCharacter = &c->m_Cur;
				m_Snap.m_pLocalPrevCharacter = &c->m_Prev;
				m_LocalCharacterPos = vec2(m_Snap.m_pLocalCharacter->m_X, m_Snap.m_pLocalCharacter->m_Y);
			}
		}
		else if(Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_CHARACTER, m_LocalClientID))
		{
			// player died
			m_pControls->OnPlayerDeath();
		}
	}
	else
	{
		m_Snap.m_SpecInfo.m_Active = true;
		if (m_DemoSpecMode == SPEC_PLAYER)
		{
			m_Snap.m_SpecInfo.m_SpecMode = SPEC_FREEVIEW;
			m_Snap.m_SpecInfo.m_SpectatorID = -1;
		}
		else
		{
			m_Snap.m_SpecInfo.m_SpecMode = m_DemoSpecMode;
			m_Snap.m_SpecInfo.m_SpectatorID = m_DemoSpecID;
		}
	}

	// sort player infos by score
	FCompareFunc Compare = (m_GameInfo.m_GameFlags&GAMEFLAG_RACE) ? CompareTime : CompareScore;

	for(int k = 0; k < MAX_CLIENTS-1; k++) // ffs, bubblesort
	{
		for(int i = 0; i < MAX_CLIENTS-k-1; i++)
		{
			if(m_Snap.m_aInfoByScore[i+1].m_pPlayerInfo && (!m_Snap.m_aInfoByScore[i].m_pPlayerInfo ||
				Compare(m_Snap.m_aInfoByScore[i].m_pPlayerInfo, m_Snap.m_aInfoByScore[i+1].m_pPlayerInfo)))
			{
				CPlayerInfoItem Tmp = m_Snap.m_aInfoByScore[i];
				m_Snap.m_aInfoByScore[i] = m_Snap.m_aInfoByScore[i+1];
				m_Snap.m_aInfoByScore[i+1] = Tmp;
			}
		}
	}

	// calc some player stats
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(!m_Snap.m_paPlayerInfos[i])
			continue;

		// count not ready players
		if(m_Snap.m_pGameData && (m_Snap.m_pGameData->m_GameStateFlags&(GAMESTATEFLAG_STARTCOUNTDOWN|GAMESTATEFLAG_PAUSED|GAMESTATEFLAG_WARMUP)) &&
			m_Snap.m_pGameData->m_GameStateEndTick == 0 && m_aClients[i].m_Team != TEAM_SPECTATORS && !(m_Snap.m_paPlayerInfos[i]->m_PlayerFlags&PLAYERFLAG_READY))
			m_Snap.m_NotReadyCount++;

		// count alive players per team
		if((m_GameInfo.m_GameFlags&GAMEFLAG_SURVIVAL) && m_aClients[i].m_Team != TEAM_SPECTATORS && !(m_Snap.m_paPlayerInfos[i]->m_PlayerFlags&PLAYERFLAG_DEAD))
			m_Snap.m_AliveCount[m_aClients[i].m_Team]++;
	}
}

void CGameClient::OnDemoRecSnap()
{
	// add client info
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(!m_aClients[i].m_Active)
			continue;

		CNetObj_De_ClientInfo *pClientInfo = static_cast<CNetObj_De_ClientInfo *>(Client()->SnapNewItem(NETOBJTYPE_DE_CLIENTINFO, i, sizeof(CNetObj_De_ClientInfo)));
		if(!pClientInfo)
			return;

		pClientInfo->m_Local = i==m_LocalClientID ? 1 : 0;
		pClientInfo->m_Team = m_aClients[i].m_Team;
		StrToInts(pClientInfo->m_aName, 4, m_aClients[i].m_aName);
		StrToInts(pClientInfo->m_aClan, 3, m_aClients[i].m_aClan);
		pClientInfo->m_Country = m_aClients[i].m_Country;

		for(int p = 0; p < NUM_SKINPARTS; p++)
		{
			StrToInts(pClientInfo->m_aaSkinPartNames[p], 6, m_aClients[i].m_aaSkinPartNames[p]);
			pClientInfo->m_aUseCustomColors[p] = m_aClients[i].m_aUseCustomColors[p];
			pClientInfo->m_aSkinPartColors[p] = m_aClients[i].m_aSkinPartColors[p];
		}
	}

	// add game info
	CNetObj_De_GameInfo *pGameInfo = static_cast<CNetObj_De_GameInfo *>(Client()->SnapNewItem(NETOBJTYPE_DE_GAMEINFO, 0, sizeof(CNetObj_De_GameInfo)));
	if(!pGameInfo)
		return;

	pGameInfo->m_GameFlags = m_GameInfo.m_GameFlags;
	pGameInfo->m_ScoreLimit = m_GameInfo.m_ScoreLimit;
	pGameInfo->m_TimeLimit = m_GameInfo.m_TimeLimit;
	pGameInfo->m_MatchNum = m_GameInfo.m_MatchNum;
	pGameInfo->m_MatchCurrent = m_GameInfo.m_MatchCurrent;
}

vec2 CGameClient::GetCharPos(int ClientID, bool Predicted) const
{
	return mix(
		vec2(m_Snap.m_aCharacters[ClientID].m_Prev.m_X, m_Snap.m_aCharacters[ClientID].m_Prev.m_Y),
		vec2(m_Snap.m_aCharacters[ClientID].m_Cur.m_X, m_Snap.m_aCharacters[ClientID].m_Cur.m_Y),
		Client()->IntraGameTick()
	);
}

void CGameClient::CClientData::UpdateRenderInfo(CGameClient *pGameClient, int ClientID, bool UpdateSkinInfo)
{
}

void CGameClient::CClientData::Reset(CGameClient *pGameClient, int ClientID)
{
	m_aName[0] = 0;
	m_aClan[0] = 0;
	m_Country = -1;
	m_Team = 0;
	m_Angle = 0;
	m_Emoticon = 0;
	m_EmoticonStart = -1;
	m_Active = false;
	m_ChatIgnore = false;
	m_Friend = false;
	m_Evolved.m_Tick = -1;
}

void CGameClient::DoEnterMessage(const char *pName, int ClientID, int Team)
{
	char aBuf[128], aLabel[64];
	GetPlayerLabel(aLabel, sizeof(aLabel), ClientID, pName);
	switch(GetStrTeam(Team, m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS))
	{
	case STR_TEAM_GAME: str_format(aBuf, sizeof(aBuf), Localize("'%s' entered and joined the game"), aLabel); break;
	case STR_TEAM_RED: str_format(aBuf, sizeof(aBuf), Localize("'%s' entered and joined the red team"), aLabel); break;
	case STR_TEAM_BLUE: str_format(aBuf, sizeof(aBuf), Localize("'%s' entered and joined the blue team"), aLabel); break;
	case STR_TEAM_SPECTATORS: str_format(aBuf, sizeof(aBuf), Localize("'%s' entered and joined the spectators"), aLabel); break;
	}
	m_pChat->AddLine(aBuf);
}

void CGameClient::DoLeaveMessage(const char *pName, int ClientID, const char *pReason)
{
	char aBuf[128], aLabel[64];
	GetPlayerLabel(aLabel, sizeof(aLabel), ClientID, pName);
	if(pReason[0])
		str_format(aBuf, sizeof(aBuf), Localize("'%s' has left the game (%s)"), aLabel, pReason);
	else
		str_format(aBuf, sizeof(aBuf), Localize("'%s' has left the game"), aLabel);
	m_pChat->AddLine(aBuf);
}

void CGameClient::DoTeamChangeMessage(const char *pName, int ClientID, int Team)
{
	char aBuf[128];
	char aLabel[64];
	GetPlayerLabel(aLabel, sizeof(aLabel), ClientID, pName);
	switch(GetStrTeam(Team, m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS))
	{
	case STR_TEAM_GAME: str_format(aBuf, sizeof(aBuf), Localize("'%s' joined the game"), aLabel); break;
	case STR_TEAM_RED: str_format(aBuf, sizeof(aBuf), Localize("'%s' joined the red team"), aLabel); break;
	case STR_TEAM_BLUE: str_format(aBuf, sizeof(aBuf), Localize("'%s' joined the blue team"), aLabel); break;
	case STR_TEAM_SPECTATORS: str_format(aBuf, sizeof(aBuf), Localize("'%s' joined the spectators"), aLabel); break;
	}
	m_pChat->AddLine(aBuf);
}

void CGameClient::SendSwitchTeam(int Team)
{
	CNetMsg_Cl_SetTeam Msg;
	Msg.m_Team = Team;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

void CGameClient::SendStartInfo()
{
	CNetMsg_Cl_StartInfo Msg;
	Msg.m_pName = Config()->m_PlayerName;
	Msg.m_pClan = Config()->m_PlayerClan;
	Msg.m_Country = Config()->m_PlayerCountry;
	for(int p = 0; p < NUM_SKINPARTS; p++)
	{
		Msg.m_apSkinPartNames[p] = "greensward";
		Msg.m_aUseCustomColors[p] = 0;
		Msg.m_aSkinPartColors[p] = 0;
	}
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_FLUSH);
}

void CGameClient::SendKill()
{
	CNetMsg_Cl_Kill Msg;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

void CGameClient::SendReadyChange()
{
	CNetMsg_Cl_ReadyChange Msg;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

void CGameClient::SendSkinChange()
{
	CNetMsg_Cl_SkinChange Msg;
	for(int p = 0; p < NUM_SKINPARTS; p++)
	{
		Msg.m_apSkinPartNames[p] = "greensward";
		Msg.m_aUseCustomColors[p] = 0;
		Msg.m_aSkinPartColors[p] = 0;
	}
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD|MSGFLAG_FLUSH);
	m_LastSkinChangeTime = Client()->LocalTime();
}

int CGameClient::GetClientID(const char *pName)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_aClients[i].m_Active || i == m_LocalClientID) // skip local user
			continue;

		if(!str_comp(m_aClients[i].m_aName, pName))
			return i;
	}

	return -1;
}

IGameClient *CreateGameClient()
{
	return new CGameClient();
}
