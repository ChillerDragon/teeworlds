/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <generated/client_data.h>

#include <base/system.h>

#include <engine/shared/ringbuffer.h>
#include <engine/shared/config.h>


#include <engine/storage.h>
#include <engine/keys.h>
#include <engine/console.h>

#include <game/client/ui.h>

#include <game/version.h>

#include <game/client/components/controls.h>
#include <game/client/components/menus.h>

#include "console.h"

enum
{
	CONSOLE_CLOSED,
	CONSOLE_OPENING,
	CONSOLE_OPEN,
	CONSOLE_CLOSING,
};

CGameConsole::CInstance::CInstance(int Type)
{
}

void CGameConsole::CInstance::Init(CGameConsole *pGameConsole)
{
};

void CGameConsole::CInstance::ClearBacklog()
{
}

void CGameConsole::CInstance::ClearHistory()
{
}

void CGameConsole::CInstance::ExecuteLine(const char *pLine)
{
}

void CGameConsole::CInstance::PossibleCommandsCompleteCallback(int Index, const char *pStr, void *pUser)
{
}

void CGameConsole::CInstance::PossibleArgumentsCompleteCallback(int Index, const char *pStr, void *pUser)
{
}

void CGameConsole::CInstance::PrintLine(const char *pLine, bool Highlighted)
{
}

CGameConsole::CGameConsole()
: m_LocalConsole(CONSOLETYPE_LOCAL), m_RemoteConsole(CONSOLETYPE_REMOTE)
{
}

float CGameConsole::TimeNow()
{
	return 0.0f;
}

CGameConsole::CInstance *CGameConsole::CurrentConsole()
{
	return &m_LocalConsole;
}

void CGameConsole::OnReset()
{
}

struct CCompletionOptionRenderInfo
{
	CGameConsole *m_pSelf;
	const char *m_pCurrentCmd;
	int m_WantedCompletion;
	int m_EnumCount;
	float m_Offset;
	float *m_pOffsetChange;
	float m_Width;
	float m_TotalWidth;
};

void CGameConsole::PossibleCommandsRenderCallback(int Index, const char *pStr, void *pUser)
{
}

void CGameConsole::OnRender()
{
}


void CGameConsole::Toggle(int Type)
{
}

void CGameConsole::Dump(int Type)
{
}

void CGameConsole::ConToggleLocalConsole(IConsole::IResult *pResult, void *pUserData)
{
}

void CGameConsole::ConToggleRemoteConsole(IConsole::IResult *pResult, void *pUserData)
{
}

void CGameConsole::ConClearLocalConsole(IConsole::IResult *pResult, void *pUserData)
{
}

void CGameConsole::ConClearRemoteConsole(IConsole::IResult *pResult, void *pUserData)
{
}

void CGameConsole::ConDumpLocalConsole(IConsole::IResult *pResult, void *pUserData)
{
}

void CGameConsole::ConDumpRemoteConsole(IConsole::IResult *pResult, void *pUserData)
{
}

void CGameConsole::ClientConsolePrintCallback(const char *pStr, void *pUserData, bool Highlighted)
{
}

void CGameConsole::ConchainConsoleOutputLevelUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
}

bool CGameConsole::IsConsoleActive()
{
	return false;
}

void CGameConsole::PrintLine(int Type, const char *pLine)
{
}

void CGameConsole::OnConsoleInit()
{
}

void CGameConsole::OnStateChange(int NewState, int OldState)
{
}
