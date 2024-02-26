/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */


#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>
#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <game/client/render.h>

#include "menus.h"
#include "controls.h"
#include "camera.h"
#include "hud.h"
#include "voting.h"
#include "binds.h"
#include "motd.h"
#include "scoreboard.h"
#include "stats.h"

CHud::CHud()
{
	// won't work if zero
	m_AverageFPS = 1.0f;

	m_WarmupHideTick = 0;
	m_CheckpointTime = 0;
}

void CHud::OnReset()
{
	m_WarmupHideTick = 0;
	m_CheckpointTime = 0;
}

bool CHud::IsLargeWarmupTimerShown()
{
	return !m_pClient->m_pMotd->IsActive()
		&& !m_pClient->m_pScoreboard->IsActive()
		&& !m_pClient->m_pStats->IsActive()
		&& (m_WarmupHideTick == 0 || (time_get() - m_WarmupHideTick) / time_freq() < 10); // inactivity based
}

void CHud::RenderGameTimer()
{
}

void CHud::RenderPauseTimer()
{
}

void CHud::RenderStartCountdown()
{
}

void CHud::RenderNetworkIssueNotification()
{
}

void CHud::RenderDeadNotification()
{
}

void CHud::RenderScoreHud()
{
}

void CHud::RenderWarmupTimer()
{
}

void CHud::RenderFps()
{
}

void CHud::RenderConnectionWarning()
{
}

void CHud::RenderTeambalanceWarning()
{
}

void CHud::RenderVoting()
{
}

void CHud::RenderCursor()
{
}

void CHud::RenderNinjaBar(float x, float y, float Progress)
{
}

void CHud::RenderHealthAndAmmo(const CNetObj_Character *pCharacter)
{
}

void CHud::RenderSpectatorHud()
{
}

void CHud::RenderSpectatorNotification()
{
}

void CHud::RenderReadyUpNotification()
{
}

void CHud::RenderRaceTime(const CNetObj_PlayerInfoRace *pRaceInfo)
{
}

void CHud::RenderCheckpoint()
{
}

void CHud::RenderLocalTime(float x)
{
}

void CHud::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_CHECKPOINT)
	{
		CNetMsg_Sv_Checkpoint *pMsg = (CNetMsg_Sv_Checkpoint *)pRawMsg;
		m_CheckpointDiff = pMsg->m_Diff;
		m_CheckpointTime = time_get();
	}
	else if(MsgType == NETMSGTYPE_SV_KILLMSG && (m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_RACE))
	{
		// reset checkpoint time on death
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;
		if(pMsg->m_Victim == m_pClient->m_LocalClientID)
			m_CheckpointTime = 0;
	}
}

void CHud::OnRender()
{
}
