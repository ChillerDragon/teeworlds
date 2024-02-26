/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <engine/engine.h>


#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>
#include <game/client/localization.h>

#include <game/client/components/scoreboard.h>

#include "menus.h"
#include "chat.h"
#include "binds.h"

CChat::CChat()
{
}

void CChat::OnReset()
{
}

void CChat::OnMapLoad()
{
}

void CChat::OnRelease()
{
}

void CChat::OnStateChange(int NewState, int OldState)
{
}

void CChat::ConSay(IConsole::IResult *pResult, void *pUserData)
{
}

void CChat::ConSayTeam(IConsole::IResult *pResult, void *pUserData)
{
}

void CChat::ConSaySelf(IConsole::IResult *pResult, void *pUserData)
{
}

void CChat::ConWhisper(IConsole::IResult *pResult, void *pUserData)
{
}

void CChat::ConChat(IConsole::IResult *pResult, void *pUserData)
{
}

void CChat::ConShowChat(IConsole::IResult *pResult, void *pUserData)
{
}

void CChat::ConChatCommand(IConsole::IResult *pResult, void *pUserData)
{
}

void CChat::OnInit()
{
	m_CommandManager.Init(Console());
}

void CChat::OnConsoleInit()
{
}

void CChat::ClearChatBuffer()
{
}

void CChat::EnableMode(int Mode, const char *pText)
{
}

void CChat::Disable()
{
}

void CChat::ClearInput()
{
}

void CChat::ServerCommandCallback(IConsole::IResult *pResult, void *pContext)
{
}

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
		if(!m_CommandManager.AddCommand(pMsg->m_Name, pMsg->m_HelpText, pMsg->m_ArgsFormat, ServerCommandCallback, this))
			dbg_msg("chat_commands", "adding server chat command: name='%s' args='%s' help='%s'", pMsg->m_Name, pMsg->m_ArgsFormat, pMsg->m_HelpText);
		else
			dbg_msg("chat_commands", "failed to add command '%s'", pMsg->m_Name);

	}
	else if(MsgType == NETMSGTYPE_SV_COMMANDINFOREMOVE)
	{
		CNetMsg_Sv_CommandInfoRemove *pMsg = (CNetMsg_Sv_CommandInfoRemove *)pRawMsg;

		if(!m_CommandManager.RemoveCommand(pMsg->m_Name))
		{
			dbg_msg("chat_commands", "removed chat command: name='%s'", pMsg->m_Name);
		}
	}
}

bool CChat::IsClientIgnored(int ClientID)
{
	return true;
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

void CChat::OnRender()
{
}

void CChat::Say(int Mode, const char *pLine)
{
	m_LastChatSend = time_get();

	// send chat message
	CNetMsg_Cl_Say Msg;
	Msg.m_Mode = Mode;
	Msg.m_Target = Mode==CHAT_WHISPER ? m_WhisperTarget : -1;
	Msg.m_pMessage = pLine;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

bool CChat::IsTypingCommand() const
{
	return false;
}

// chat commands handlers
void CChat::HandleCommands(float x, float y, float w)
{
}

bool CChat::ExecuteCommand()
{
	return false;
}

bool CChat::CompleteCommand()
{
	return true;
}

// callback functions for commands
void CChat::Com_All(IConsole::IResult *pResult, void *pContext)
{
}

void CChat::Com_Team(IConsole::IResult *pResult, void *pContext)
{
}

void CChat::Com_Reply(IConsole::IResult *pResult, void *pContext)
{
}

void CChat::Com_Whisper(IConsole::IResult *pResult, void *pContext)
{
}

void CChat::Com_Mute(IConsole::IResult *pResult, void *pContext)
{
}

void CChat::Com_Befriend(IConsole::IResult *pResult, void *pContext)
{
}


int CChat::FilterChatCommands(const char *pLine)
{
	return 0;
}

int CChat::GetFirstActiveCommand()
{
	return -1;
}

int CChat::NextActiveCommand(int *pIndex)
{
	return 0;
}

int CChat::PreviousActiveCommand(int *pIndex)
{
	return 0;
}

int CChat::GetActiveCountRange(int i, int j)
{
	return 0;
}
