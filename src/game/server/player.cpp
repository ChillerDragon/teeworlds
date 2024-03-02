/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include "gamecontext.h"
#include "gamecontroller.h"
#include "player.h"

IServer *CPlayer::Server() const { return m_pGameServer->Server(); }

CPlayer::CPlayer(CGameContext *pGameServer, int ClientID, bool Dummy, bool AsSpec)
{
	m_pGameServer = pGameServer;
	m_ActiveSpecSwitch = 0;
	m_InactivityTickCounter = 0;
	m_Dummy = Dummy;
	m_DeadSpecMode = false;
	m_Spawning = false;
	mem_zero(&m_Latency, sizeof(m_Latency));
}

CPlayer::~CPlayer()
{
}

void CPlayer::Snap(int SnappingClient)
{
	CNetObj_PlayerInfo *pPlayerInfo = static_cast<CNetObj_PlayerInfo *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFO, m_ClientID, sizeof(CNetObj_PlayerInfo)));
	if(!pPlayerInfo)
		return;

	pPlayerInfo->m_PlayerFlags = m_PlayerFlags&PLAYERFLAG_CHATTING;
	pPlayerInfo->m_PlayerFlags |= PLAYERFLAG_ADMIN;
	pPlayerInfo->m_PlayerFlags |= PLAYERFLAG_READY;
	pPlayerInfo->m_PlayerFlags |= PLAYERFLAG_DEAD;
	if(SnappingClient != -1 && (m_Team == TEAM_SPECTATORS || m_DeadSpecMode) && (SnappingClient == m_SpectatorID))
		pPlayerInfo->m_PlayerFlags |= PLAYERFLAG_WATCHING;

	pPlayerInfo->m_Latency = SnappingClient == -1 ? m_Latency.m_Min : GameServer()->m_apPlayers[SnappingClient]->m_aActLatency[m_ClientID];
	pPlayerInfo->m_Score = m_Score;

	if(m_ClientID == SnappingClient && (m_Team == TEAM_SPECTATORS || m_DeadSpecMode))
	{
		CNetObj_SpectatorInfo *pSpectatorInfo = static_cast<CNetObj_SpectatorInfo *>(Server()->SnapNewItem(NETOBJTYPE_SPECTATORINFO, m_ClientID, sizeof(CNetObj_SpectatorInfo)));
		if(!pSpectatorInfo)
			return;

		pSpectatorInfo->m_SpecMode = m_SpecMode;
		pSpectatorInfo->m_SpectatorID = m_SpectatorID;
		pSpectatorInfo->m_X = m_ViewPos.x;
		pSpectatorInfo->m_Y = m_ViewPos.y;
	}
}
