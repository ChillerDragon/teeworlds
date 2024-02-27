/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/serverbrowser.h>
#include <game/client/animstate.h>
#include <game/client/components/menus.h>
#include <game/client/gameclient.h>
#include <generated/client_data.h>
#include "stats.h"

static const vec4 s_aWeaponColors[] =
{
	vec4(201/255.0f, 197/255.0f, 205/255.0f, 1.0f),
	vec4(156/255.0f, 158/255.0f, 100/255.0f, 1.0f),
	vec4(98/255.0f, 80/255.0f, 46/255.0f, 1.0f),
	vec4(163/255.0f, 51/255.0f, 56/255.0f, 1.0f),
	vec4(65/255.0f, 97/255.0f, 161/255.0f, 1.0f),
	vec4(182/255.0f, 137/255.0f, 40/255.0f, 1.0f),
};

CStats::CStats()
{
}

void CStats::CPlayerStats::Reset()
{
}

void CStats::OnReset()
{
}

bool CStats::IsActive() const
{
	return true;
}

void CStats::OnRelease()
{
}

void CStats::ConKeyStats(IConsole::IResult *pResult, void *pUserData)
{
}

void CStats::OnConsoleInit()
{
}

void CStats::OnMessage(int MsgType, void *pRawMsg)
{
	if(m_pClient->m_SuppressEvents)
		return;

	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;

		if(pMsg->m_Weapon != -3)	// team switch
			m_aStats[pMsg->m_Victim].m_Deaths++;
		m_aStats[pMsg->m_Victim].m_CurrentSpree = 0;
		if(pMsg->m_Weapon >= 0)
			m_aStats[pMsg->m_Victim].m_aDeathsFrom[pMsg->m_Weapon]++;
		if((pMsg->m_ModeSpecial & 1) && (pMsg->m_Weapon != -3))
			m_aStats[pMsg->m_Victim].m_DeathsCarrying++;
		if(pMsg->m_Victim != pMsg->m_Killer)
		{
			m_aStats[pMsg->m_Killer].m_Frags++;
			m_aStats[pMsg->m_Killer].m_CurrentSpree++;

			if(m_aStats[pMsg->m_Killer].m_CurrentSpree > m_aStats[pMsg->m_Killer].m_BestSpree)
				m_aStats[pMsg->m_Killer].m_BestSpree = m_aStats[pMsg->m_Killer].m_CurrentSpree;
			if(pMsg->m_Weapon >= 0)
				m_aStats[pMsg->m_Killer].m_aFragsWith[pMsg->m_Weapon]++;
			if(pMsg->m_ModeSpecial & 1)
				m_aStats[pMsg->m_Killer].m_CarriersKilled++;
			if(pMsg->m_ModeSpecial & 2)
				m_aStats[pMsg->m_Killer].m_KillsCarrying++;
		}
		else if(pMsg->m_Weapon != -3)
			m_aStats[pMsg->m_Victim].m_Suicides++;
	}
}

void CStats::OnRender()
{
}

void CStats::UpdatePlayTime(int Ticks)
{
}

void CStats::OnMatchStart()
{
}

void CStats::OnFlagGrab(int ClientID)
{
}

void CStats::OnFlagCapture(int ClientID)
{
}

void CStats::OnPlayerEnter(int ClientID, int Team)
{
}

void CStats::OnPlayerLeave(int ClientID)
{
}
