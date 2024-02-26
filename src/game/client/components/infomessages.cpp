/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */


#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include "infomessages.h"

#include "chat.h"
#include "skins.h"

void CInfoMessages::OnReset()
{
}

void CInfoMessages::AddInfoMsg(int Type, CInfoMsg NewMsg)
{
}

void CInfoMessages::OnMessage(int MsgType, void *pRawMsg)
{
	if(m_pClient->m_SuppressEvents)
		return;

	bool Race = m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_RACE;

	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		if(Race && m_pClient->m_Snap.m_pGameDataRace && m_pClient->m_Snap.m_pGameDataRace->m_RaceFlags&RACEFLAG_HIDE_KILLMSG)
			return;

		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;
		dbg_msg("kill", "killer=%d", pMsg->m_Killer);
	}
	else if(MsgType == NETMSGTYPE_SV_RACEFINISH && Race)
	{
		CNetMsg_Sv_RaceFinish *pMsg = (CNetMsg_Sv_RaceFinish *)pRawMsg;

		char aBuf[256];
		char aTime[32];
		char aLabel[64];

		FormatTime(aTime, sizeof(aTime), pMsg->m_Time, m_pClient->RacePrecision());
		m_pClient->GetPlayerLabel(aLabel, sizeof(aLabel), pMsg->m_ClientID, m_pClient->m_aClients[pMsg->m_ClientID].m_aName);

		str_format(aBuf, sizeof(aBuf), "%2d: %s: finished in %s", pMsg->m_ClientID, m_pClient->m_aClients[pMsg->m_ClientID].m_aName, aTime);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "race", aBuf);

		if(pMsg->m_RecordPersonal || pMsg->m_RecordServer)
		{
			if(pMsg->m_RecordServer)
				str_format(aBuf, sizeof(aBuf), Localize("'%s' has set a new map record: %s"), aLabel, aTime);
			else // m_RecordPersonal
				str_format(aBuf, sizeof(aBuf), Localize("'%s' has set a new personal record: %s"), aLabel, aTime);

			if(pMsg->m_Diff < 0)
			{
				char aImprovement[64];
				char aDiff[32];
				FormatTimeDiff(aDiff, sizeof(aDiff), absolute(pMsg->m_Diff), m_pClient->RacePrecision(), false);
				str_format(aImprovement, sizeof(aImprovement), Localize(" (%s seconds faster)"), aDiff);
				str_append(aBuf, aImprovement, sizeof(aBuf));
			}

			m_pClient->m_pChat->AddLine(aBuf);
		}

		if(m_pClient->m_Snap.m_pGameDataRace && m_pClient->m_Snap.m_pGameDataRace->m_RaceFlags&RACEFLAG_FINISHMSG_AS_CHAT)
		{
			if(!pMsg->m_RecordPersonal && !pMsg->m_RecordServer) // don't print the time twice
			{
				str_format(aBuf, sizeof(aBuf), Localize("'%s' finished in: %s"), aLabel, aTime);
				m_pClient->m_pChat->AddLine(aBuf);
			}
		}
		else
		{
			dbg_msg("info", "INFOMSG_FINISH");
		}
	}
}

void CInfoMessages::OnRender()
{
}

void CInfoMessages::RenderKillMsg(CInfoMsg *pInfoMsg, float x, float y) const
{
}

void CInfoMessages::RenderFinishMsg(CInfoMsg *pInfoMsg, float x, float y) const
{
}
