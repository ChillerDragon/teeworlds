/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>

#include "chat.h"

void CChat::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_CHAT)
	{
		CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)pRawMsg;
		if(pMsg->m_Mode == CHAT_WHISPER)
			return;
		AddLine(pMsg->m_pMessage, pMsg->m_ClientID, pMsg->m_Mode, pMsg->m_TargetID);
	}
	else if(MsgType == NETMSGTYPE_SV_COMMANDINFO)
	{
		CNetMsg_Sv_CommandInfo *pMsg = (CNetMsg_Sv_CommandInfo *)pRawMsg;
		dbg_msg("chat_commands", "adding server chat command: name='%s' args='%s' help='%s'", pMsg->m_Name, pMsg->m_ArgsFormat, pMsg->m_HelpText);
	}
	else if(MsgType == NETMSGTYPE_SV_COMMANDINFOREMOVE)
	{
		CNetMsg_Sv_CommandInfoRemove *pMsg = (CNetMsg_Sv_CommandInfoRemove *)pRawMsg;

		dbg_msg("chat_commands", "removed chat command: name='%s'", pMsg->m_Name);
	}
}

void CChat::AddLine(const char *pLine, int ClientID, int Mode, int TargetID)
{
	dbg_msg(GetModeName(Mode), "%2d: %s: %s", ClientID, "name", pLine);
}

const char *CChat::GetModeName(int Mode) const
{
	switch(Mode)
	{
		case CHAT_ALL: return "all";
		case CHAT_WHISPER: return "whisper";
		case CHAT_TEAM: return "team";
		default: return "";
	}
}

void CChat::Say(int Mode, const char *pLine)
{
	// send chat message
	CNetMsg_Cl_Say Msg;
	Msg.m_Mode = Mode;
	Msg.m_Target = Mode==CHAT_WHISPER ? m_WhisperTarget : -1;
	Msg.m_pMessage = pLine;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}
