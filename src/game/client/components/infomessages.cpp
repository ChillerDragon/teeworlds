/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/textrender.h>
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

		// unpack messages
		CInfoMsg Kill;
		Kill.m_Player1ID = pMsg->m_Victim;
		if(Config()->m_ClShowsocial)
		{
			Kill.m_Player1NameCursor.m_FontSize = 36.0f;
		}

		Kill.m_Player1RenderInfo = m_pClient->m_aClients[Kill.m_Player1ID].m_RenderInfo;

		Kill.m_Player2ID = pMsg->m_Killer;
		if(Kill.m_Player2ID >= 0)
		{
			if(Config()->m_ClShowsocial)
			{
				Kill.m_Player2NameCursor.m_FontSize = 36.0f;
			}

			Kill.m_Player2RenderInfo = m_pClient->m_aClients[Kill.m_Player2ID].m_RenderInfo;
		}
		else
		{
			bool IsTeamplay = (m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS) != 0;
			int KillerTeam = - 1 - Kill.m_Player2ID;
			int Skin = m_pClient->m_pSkins->Find("dummy", false);
			if(Skin != -1)
			{
				const CSkins::CSkin *pDummy = m_pClient->m_pSkins->Get(Skin);
				for(int p = 0; p < NUM_SKINPARTS; p++)
				{
					Kill.m_Player2RenderInfo.m_aTextures[p] = pDummy->m_apParts[p]->m_OrgTexture;
					if(IsTeamplay)
					{
						int ColorVal = m_pClient->m_pSkins->GetTeamColor(0, 0x000000, KillerTeam, p);
						Kill.m_Player2RenderInfo.m_aColors[p] = m_pClient->m_pSkins->GetColorV4(ColorVal, p==SKINPART_MARKING);
					}
					else
						Kill.m_Player2RenderInfo.m_aColors[p] = m_pClient->m_pSkins->GetColorV4(0x000000, p==SKINPART_MARKING);
					Kill.m_Player2RenderInfo.m_aColors[p].a *= .5f;
				}
				Kill.m_Player2RenderInfo.m_Size = 64.0f;
			}
		}

		Kill.m_Weapon = pMsg->m_Weapon;
		Kill.m_ModeSpecial = pMsg->m_ModeSpecial;
		Kill.m_FlagCarrierBlue = m_pClient->m_Snap.m_pGameDataFlag ? m_pClient->m_Snap.m_pGameDataFlag->m_FlagCarrierBlue : -1;

		AddInfoMsg(INFOMSG_KILL, Kill);
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
			CInfoMsg Finish;
			Finish.m_Player1ID = pMsg->m_ClientID;
			Finish.m_Player1RenderInfo = m_pClient->m_aClients[Finish.m_Player1ID].m_RenderInfo;

			Finish.m_TimeCursor.m_FontSize = 36.0f;

			if(Config()->m_ClShowsocial)
			{
				Finish.m_Player1NameCursor.m_FontSize = 36.0f;
			}

			FormatTimeDiff(aTime, sizeof(aTime), pMsg->m_Diff, m_pClient->RacePrecision());
			str_format(aBuf, sizeof(aBuf), "(%s)", aTime);
			Finish.m_DiffCursor.m_FontSize = 36.0f;

			Finish.m_Time = pMsg->m_Time;
			Finish.m_Diff = pMsg->m_Diff;
			Finish.m_RecordPersonal = pMsg->m_RecordPersonal;
			Finish.m_RecordServer = pMsg->m_RecordServer;

			AddInfoMsg(INFOMSG_FINISH, Finish);
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
