/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */


#include <generated/protocol.h>
#include <generated/client_data.h>

#include "infomessages.h"

#include "chat.h"

void CInfoMessages::OnMessage(int MsgType, void *pRawMsg)
{
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

		str_format(aTime, sizeof(aTime), "%d", pMsg->m_Time);

		str_format(aBuf, sizeof(aBuf), "%2d: %s: finished in %s", pMsg->m_ClientID, m_pClient->m_aClients[pMsg->m_ClientID].m_aName, aTime);
		dbg_msg("race", "%s", aBuf);

		if(m_pClient->m_Snap.m_pGameDataRace && m_pClient->m_Snap.m_pGameDataRace->m_RaceFlags&RACEFLAG_FINISHMSG_AS_CHAT)
		{
			if(!pMsg->m_RecordPersonal && !pMsg->m_RecordServer) // don't print the time twice
			{
				str_format(aBuf, sizeof(aBuf), "'%s' finished in: %s", m_pClient->m_aClients[pMsg->m_ClientID].m_aName, aTime);
				dbg_msg("chat", "%s", aBuf);
			}
		}
		else
		{
			dbg_msg("info", "INFOMSG_FINISH");
		}
	}
}
