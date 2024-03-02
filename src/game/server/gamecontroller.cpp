/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include "gamecontext.h"
#include "gamecontroller.h"
#include "player.h"


IGameController::IGameController(CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;
	m_pServer = m_pGameServer->Server();
}

void IGameController::Snap(int SnappingClient)
{
	CNetObj_GameData *pGameData = static_cast<CNetObj_GameData *>(Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData)));
	if(!pGameData)
		return;

	pGameData->m_GameStartTick = 0;
	pGameData->m_GameStateFlags = 0;
	// pGameData->m_GameStateFlags |= GAMESTATEFLAG_WARMUP;
	// pGameData->m_GameStateFlags |= GAMESTATEFLAG_STARTCOUNTDOWN|GAMESTATEFLAG_PAUSED;
	// pGameData->m_GameStateFlags |= GAMESTATEFLAG_PAUSED;
	// pGameData->m_GameStateFlags |= GAMESTATEFLAG_ROUNDOVER;
	// pGameData->m_GameStateFlags |= GAMESTATEFLAG_GAMEOVER;
	// pGameData->m_GameStateFlags |= GAMESTATEFLAG_SUDDENDEATH;
	pGameData->m_GameStateEndTick = 0; // no timer/infinite = 0, on end = GameEndTick, otherwise = GameStateEndTick

	// teamplay
	CNetObj_GameDataTeam *pGameDataTeam = static_cast<CNetObj_GameDataTeam *>(Server()->SnapNewItem(NETOBJTYPE_GAMEDATATEAM, 0, sizeof(CNetObj_GameDataTeam)));
	if(!pGameDataTeam)
		return;
	pGameDataTeam->m_TeamscoreRed = 420;
	pGameDataTeam->m_TeamscoreBlue = 69;

	// demo recording
	if(SnappingClient == -1)
	{
		CNetObj_De_GameInfo *pGameInfo = static_cast<CNetObj_De_GameInfo *>(Server()->SnapNewItem(NETOBJTYPE_DE_GAMEINFO, 0, sizeof(CNetObj_De_GameInfo)));
		if(!pGameInfo)
			return;

		pGameInfo->m_GameFlags = 0;
		pGameInfo->m_ScoreLimit = 69;
		pGameInfo->m_TimeLimit = 0;
		pGameInfo->m_MatchNum = 0;
		pGameInfo->m_MatchCurrent = 0;
	}
}

void IGameController::UpdateGameInfo(int ClientID)
{
	CNetMsg_Sv_GameInfo GameInfoMsg;
	GameInfoMsg.m_GameFlags = 0;
	GameInfoMsg.m_ScoreLimit = 69;
	GameInfoMsg.m_TimeLimit = 0;
	GameInfoMsg.m_MatchNum = 0;
	GameInfoMsg.m_MatchCurrent = 0;

	CNetMsg_Sv_GameInfo GameInfoMsgNoRace = GameInfoMsg;
	GameInfoMsgNoRace.m_GameFlags &= ~GAMEFLAG_RACE;

	if(ClientID == -1)
	{
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(!GameServer()->m_apPlayers[i] || !Server()->ClientIngame(i))
				continue;

			CNetMsg_Sv_GameInfo *pInfoMsg = (Server()->GetClientVersion(i) < CGameContext::MIN_RACE_CLIENTVERSION) ? &GameInfoMsgNoRace : &GameInfoMsg;
			Server()->SendPackMsg(pInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);
		}
	}
	else
	{
		CNetMsg_Sv_GameInfo *pInfoMsg = (Server()->GetClientVersion(ClientID) < CGameContext::MIN_RACE_CLIENTVERSION) ? &GameInfoMsgNoRace : &GameInfoMsg;
		Server()->SendPackMsg(pInfoMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID);
	}
}

void IGameController::SendTeamChange(CPlayer *pPlayer, int Team, bool DoChatMsg)
{
	CNetMsg_Sv_Team Msg;
	Msg.m_ClientID = 1;
	Msg.m_Team = TEAM_RED;
	Msg.m_Silent = 0;
	Msg.m_CooldownTick = Server()->Tick() + Server()->TickSpeed() * 3;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
}
