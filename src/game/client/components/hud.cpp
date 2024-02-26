/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/textrender.h>
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
	float x = 300.0f*Graphics()->ScreenAspect()/2.0f;

	int Time = 0;
	if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH)
		Time = 0;
	else if(m_pClient->m_GameInfo.m_TimeLimit && !(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_WARMUP))
	{
		Time = m_pClient->m_GameInfo.m_TimeLimit*60 - ((Client()->GameTick()-m_pClient->m_Snap.m_pGameData->m_GameStartTick)/Client()->GameTickSpeed());

		if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&(GAMESTATEFLAG_ROUNDOVER|GAMESTATEFLAG_GAMEOVER))
			Time = 0;
	}
	else if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&(GAMESTATEFLAG_ROUNDOVER|GAMESTATEFLAG_GAMEOVER))
		Time = m_pClient->m_Snap.m_pGameData->m_GameStateEndTick/Client()->GameTickSpeed();
	else
		Time = (Client()->GameTick()-m_pClient->m_Snap.m_pGameData->m_GameStartTick)/Client()->GameTickSpeed();

	static CTextCursor s_Cursor(10.0f);
	s_Cursor.MoveTo(x, 2.0f);
	s_Cursor.m_Align = TEXTALIGN_TC;
	s_Cursor.Reset(Time);

	char aBuf[32];
	str_format(aBuf, sizeof(aBuf), "%d:%02d", Time/60, Time%60);

	// last 60 sec red, last 10 sec blink
	float Alpha = 1.0f;
	if(m_pClient->m_GameInfo.m_TimeLimit && Time <= 60 && !(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_WARMUP))
	{
		Alpha = Time <= 10 && (2*time_get()/time_freq()) % 2 ? 0.5f : 1.0f;
		TextRender()->TextColor(1.0f, 0.25f, 0.25f, 1.0f);
	}
	TextRender()->TextDeferred(&s_Cursor, aBuf, -1.0f);
	TextRender()->DrawTextOutlined(&s_Cursor, Alpha);
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

	if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH)
	{
		static CTextCursor s_SuddenDeathCursor(12.0f);
		s_SuddenDeathCursor.MoveTo(x, 14.0f);
		s_SuddenDeathCursor.m_Align = TEXTALIGN_TC;
		s_SuddenDeathCursor.Reset(g_Localization.Version());
		const char *pText = Localize("Sudden Death");
		TextRender()->TextOutlined(&s_SuddenDeathCursor, pText, -1);
	}
}

void CHud::RenderPauseTimer()
{
	if((m_pClient->m_Snap.m_pGameData->m_GameStateFlags&(GAMESTATEFLAG_STARTCOUNTDOWN|GAMESTATEFLAG_PAUSED)) == GAMESTATEFLAG_PAUSED)
	{
		char aBuf[256];
		const char *pText = Localize("Game paused");
		static CTextCursor s_GamePausedCursor;
		s_GamePausedCursor.MoveTo(150 * Graphics()->ScreenAspect(), 50);
		s_GamePausedCursor.m_Align = TEXTALIGN_TC;
		s_GamePausedCursor.m_FontSize = 20.0f;
		s_GamePausedCursor.Reset(g_Localization.Version());
		TextRender()->TextOutlined(&s_GamePausedCursor, pText, -1);

		static CTextCursor s_Cursor;
		s_Cursor.MoveTo(150 * Graphics()->ScreenAspect(), 75);
		s_Cursor.m_Align = TEXTALIGN_TC;
		s_Cursor.m_FontSize = 16.0f;

		if(m_pClient->m_Snap.m_pGameData->m_GameStateEndTick == 0)
		{
			const int64 CursorVersion = g_Localization.Version() << 8 | m_pClient->m_Snap.m_NotReadyCount;

			if(m_pClient->m_Snap.m_NotReadyCount == 1)
				str_format(aBuf, sizeof(aBuf), Localize("%d player not ready"), m_pClient->m_Snap.m_NotReadyCount);
			else if(m_pClient->m_Snap.m_NotReadyCount > 1)
				str_format(aBuf, sizeof(aBuf), Localize("%d players not ready"), m_pClient->m_Snap.m_NotReadyCount);
			else
				return;
			RenderReadyUpNotification();
			s_Cursor.Reset(CursorVersion);
		}
		else
		{
			float Seconds = static_cast<float>(m_pClient->m_Snap.m_pGameData->m_GameStateEndTick-Client()->GameTick())/SERVER_TICK_SPEED;
			if(Seconds < 5)
				str_format(aBuf, sizeof(aBuf), "%.1f", Seconds);
			else
				str_format(aBuf, sizeof(aBuf), "%d", round_to_int(Seconds));
			s_Cursor.Reset();
		}

		TextRender()->TextOutlined(&s_Cursor, aBuf, -1);
	}
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
