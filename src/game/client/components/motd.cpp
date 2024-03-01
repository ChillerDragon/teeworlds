/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <generated/protocol.h>
#include <generated/client_data.h>
#include <game/client/gameclient.h>

#include "motd.h"

void CMotd::Clear()
{
	m_ServerMotdTime = 0;
}

bool CMotd::IsActive()
{
	return false;
}

void CMotd::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_MOTD)
	{
		CNetMsg_Sv_Motd *pMsg = (CNetMsg_Sv_Motd *)pRawMsg;

		// process escaping
		str_copy(m_aServerMotd, pMsg->m_pMessage, sizeof(m_aServerMotd));
		for(int i = 0; m_aServerMotd[i]; i++)
		{
			if(m_aServerMotd[i] == '\\')
			{
				if(m_aServerMotd[i+1] == 'n')
				{
					m_aServerMotd[i] = ' ';
					m_aServerMotd[i+1] = '\n';
					i++;
				}
			}
		}

		int ClMotdTime = 10;
		if(m_aServerMotd[0] && ClMotdTime)
			m_ServerMotdTime = time_get()+time_freq()*ClMotdTime;
		else
			m_ServerMotdTime = 0;
	}
}
