/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/engine.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/version.h>

#include "gameclient.h"

#include "components/broadcast.h"
#include "components/chat.h"
#include "components/controls.h"
#include "components/emoticon.h"
#include "components/infomessages.h"
#include "components/motd.h"
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

// instantiate all systems
static CInfoMessages gs_InfoMessages;
static CChat gs_Chat;
static CMotd gs_Motd;
static CBroadcast gs_Broadcast;
static CControls gs_Controls;
static CEmoticon gs_Emoticon;
static CVoting gs_Voting;
static CSpectator gs_Spectator;

CGameClient::CStack::CStack() { m_Num = 0; }
void CGameClient::CStack::Add(class CComponent *pComponent) { m_paComponents[m_Num++] = pComponent; }

const char *CGameClient::Version() const { return GAME_VERSION; }
const char *CGameClient::NetVersion() const { return GAME_NETVERSION; }
const char *CGameClient::NetVersionHashUsed() const { return GAME_NETVERSION_HASH_FORCED; }
const char *CGameClient::NetVersionHashReal() const{ return GAME_NETVERSION_HASH; }
int CGameClient::ClientVersion() const { return CLIENT_VERSION; }
const char *CGameClient::GetItemName(int Type) const { return m_NetObjHandler.GetObjName(Type); }

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
	{/*GAMEMSG_TEAM_SWAP*/ DO_CHAT, PARA_NONE, "Teams were swapped"}, // ("Teams were swapped")
	{/*GAMEMSG_SPEC_INVALIDID*/ DO_CHAT, PARA_NONE, "Invalid spectator id used"},   //!
	{/*GAMEMSG_TEAM_SHUFFLE*/ DO_CHAT, PARA_NONE, "Teams were shuffled"}, // ("Teams were shuffled")
	{/*GAMEMSG_TEAM_BALANCE*/ DO_CHAT, PARA_NONE, "Teams have been balanced"}, // ("Teams have been balanced")
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
	m_pEngine = Kernel()->RequestInterface<IEngine>();
	m_pClient = Kernel()->RequestInterface<IClient>();

	// setup pointers
	m_pBroadcast = &::gs_Broadcast;
	m_pChat = &::gs_Chat;
	m_pControls = &::gs_Controls;
	m_pMotd = &::gs_Motd;
	m_pVoting = &::gs_Voting;

	// make a list of all the systems, make sure to add them in the corrent render order
	m_All.Add(m_pControls);
	m_All.Add(m_pVoting);

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

			dbg_msg("vote", "desc=%s", pDescription);
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
					case STR_TEAM_GAME: pMsg = "All players were moved to the game"; break;
					case STR_TEAM_RED: pMsg = "All players were moved to the red team"; break;
					case STR_TEAM_BLUE: pMsg = "All players were moved to the blue team"; break;
					case STR_TEAM_SPECTATORS: pMsg = "All players were moved to the spectators"; break;
					}
					dbg_msg("client-broadcast", "%s", pMsg);
				}
				break;
			case GAMEMSG_TEAM_BALANCE_VICTIM:
				{
					const char *pMsg = "";
					switch(GetStrTeam(aParaI[0], TeamPlay))
					{
					case STR_TEAM_RED: pMsg = "You were moved to the red team due to team balancing"; break;
					case STR_TEAM_BLUE: pMsg = "You were moved to the blue team due to team balancing"; break;
					}
					dbg_msg("client-broadcast", "%s", pMsg);
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
					str_format(aBuf, sizeof(aBuf), "'%s' initiated a pause", m_aClients[ClientID].m_aName);
					m_pChat->AddLine(aBuf);
				}
				break;
			case GAMEMSG_CTF_CAPTURE:
				dbg_msg("sound", "CSounds::CHN_GLOBAL, SOUND_CTF_CAPTURE);");
				int ClientID = clamp(aParaI[1], 0, MAX_CLIENTS - 1);
				// m_pStats->OnFlagCapture(ClientID);

				float Time = aParaI[2] / (float)Client()->GameTickSpeed();
				if(Time <= 60)
				{
					if(aParaI[0])
					{
						str_format(aBuf, sizeof(aBuf), "The blue flag was captured by '%s' (%.2f seconds)", m_aClients[ClientID].m_aName, Time);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "The red flag was captured by '%s' (%.2f seconds)", m_aClients[ClientID].m_aName, Time);
					}
				}
				else
				{
					if(aParaI[0])
					{
						str_format(aBuf, sizeof(aBuf), "The blue flag was captured by '%s'", m_aClients[ClientID].m_aName);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "The red flag was captured by '%s'", m_aClients[ClientID].m_aName);
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
			pText = gs_GameMsgList[GameMsgID].m_pText;
		}

		// handle message
		switch(gs_GameMsgList[GameMsgID].m_Action)
		{
		case DO_CHAT:
			m_pChat->AddLine(pText);
			break;
		case DO_BROADCAST:
			dbg_msg("client-broadcast", "%s", pText);
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
				dbg_msg("client", "invalid clientinfo");
				return;
			}

			if(m_LocalClientID != -1 && !pMsg->m_Silent)
			{
				// DoEnterMessage(pMsg->m_pName, pMsg->m_ClientID, pMsg->m_Team);
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
			dbg_msg("client", "invalid clientdrop");
			return;
		}

		if(!pMsg->m_Silent)
		{
			// DoLeaveMessage(m_aClients[pMsg->m_ClientID].m_aName, pMsg->m_ClientID, pMsg->m_pReason);

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
			dbg_msg("client", "invalid skin info");
			return;
		}

		for(int i = 0; i < NUM_SKINPARTS; i++)
		{
			str_utf8_copy_num(m_aClients[pMsg->m_ClientID].m_aaSkinPartNames[i], pMsg->m_apSkinPartNames[i], sizeof(m_aClients[pMsg->m_ClientID].m_aaSkinPartNames[i]), MAX_SKIN_LENGTH);
			m_aClients[pMsg->m_ClientID].m_aUseCustomColors[i] = pMsg->m_aUseCustomColors[i];
			m_aClients[pMsg->m_ClientID].m_aSkinPartColors[i] = pMsg->m_aSkinPartColors[i];
		}
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
			dbg_msg("chat", "Teams were locked");
		else if(m_ServerSettings.m_TeamLock && !pMsg->m_TeamLock)
			dbg_msg("chat", "Teams were unlocked");

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

			if(pMsg->m_ClientID == m_LocalClientID)
			{
				m_TeamCooldownTick = pMsg->m_CooldownTick;
				m_TeamChangeTime = Client()->LocalTime();
			}
		}

		if(pMsg->m_Silent == 0)
		{
			// DoTeamChangeMessage(m_aClients[pMsg->m_ClientID].m_aName, pMsg->m_ClientID, pMsg->m_Team);
		}
	}
	else if(MsgId == NETMSGTYPE_SV_READYTOENTER)
	{
		// Client()->EnterGame();
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
		// CNetMsg_De_ClientEnter *pMsg = (CNetMsg_De_ClientEnter *)pRawMsg;
		// DoEnterMessage(pMsg->m_pName, pMsg->m_ClientID, pMsg->m_Team);
		// m_pStats->OnPlayerEnter(pMsg->m_ClientID, pMsg->m_Team);
	}
	else if(MsgId == NETMSGTYPE_DE_CLIENTLEAVE && Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		// CNetMsg_De_ClientLeave *pMsg = (CNetMsg_De_ClientLeave *)pRawMsg;
		// DoLeaveMessage(pMsg->m_pName, pMsg->m_ClientID, pMsg->m_pReason);
		// m_pStats->OnPlayerLeave(pMsg->m_ClientID);
	}
}

void CGameClient::OnRconLine(const char *pLine)
{
	dbg_msg("rcon", "%s", pLine);
}

void CGameClient::ProcessEvents()
{
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
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "invalidated index=%d type=%d (%s) size=%d id=%d", Index, Item.m_Type, m_NetObjHandler.GetObjName(Item.m_Type), Item.m_DataSize, Item.m_ID);
				dbg_msg("game", "%s", aBuf);
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
				// m_Snap.m_pGameData = (const CNetObj_GameData *)pData;

				// static int s_LastGameFlags = 0;
				// int GameFlags = m_Snap.m_pGameData->m_GameStateFlags;
				// if(!(s_LastGameFlags&GAMESTATEFLAG_GAMEOVER) && GameFlags&GAMESTATEFLAG_GAMEOVER)
				// else if(s_LastGameFlags&GAMESTATEFLAG_GAMEOVER && !(GameFlags&GAMESTATEFLAG_GAMEOVER))

				// if(m_Snap.m_pGameData->m_GameStartTick != m_LastGameStartTick && !(s_LastGameFlags&GAMESTATEFLAG_ROUNDOVER)
				// 	&& !(s_LastGameFlags&GAMESTATEFLAG_PAUSED) && (!(GameFlags&GAMESTATEFLAG_PAUSED) || GameFlags&GAMESTATEFLAG_STARTCOUNTDOWN))
				// {
				// 	// sts OnMatchStart();
				// }

				// // if(!(GameFlags&(GAMESTATEFLAG_PAUSED|GAMESTATEFLAG_ROUNDOVER|GAMESTATEFLAG_GAMEOVER)))
				// 	// m_pStats->UpdatePlayTime(Client()->GameTick() - Client()->PrevGameTick());

				// s_LastGameFlags = GameFlags;
				// m_LastGameStartTick = m_Snap.m_pGameData->m_GameStartTick;
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

				// m_LastFlagCarrierRed = m_Snap.m_pGameDataFlag->m_FlagCarrierRed;
				// m_LastFlagCarrierBlue = m_Snap.m_pGameDataFlag->m_FlagCarrierBlue;
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
	m_Evolved.m_Tick = -1;
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
	Msg.m_pName = "nameless tee";
	Msg.m_pClan = "0.7";
	Msg.m_Country = 0;
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

IGameClient *CreateGameClient()
{
	return new CGameClient();
}
