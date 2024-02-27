/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "spectator.h"


void CSpectator::ConKeySpectator(IConsole::IResult *pResult, void *pUserData)
{
}

void CSpectator::ConSpectate(IConsole::IResult *pResult, void *pUserData)
{
}

bool CSpectator::SpecModePossible(int SpecMode, int SpectatorID)
{
	return false;
}

void CSpectator::HandleSpectateNextPrev(int Direction)
{
}

void CSpectator::ConSpectateNext(IConsole::IResult *pResult, void *pUserData)
{
}

void CSpectator::ConSpectatePrevious(IConsole::IResult *pResult, void *pUserData)
{
}

CSpectator::CSpectator()
{
}

void CSpectator::OnConsoleInit()
{
}

void CSpectator::OnRelease()
{
	OnReset();
}

void CSpectator::OnRender()
{
}

void CSpectator::OnReset()
{
}

void CSpectator::Spectate(int SpecMode, int SpectatorID)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		m_pClient->m_DemoSpecMode = clamp(SpecMode, 0, NUM_SPECMODES-1);
		m_pClient->m_DemoSpecID = clamp(SpectatorID, -1, MAX_CLIENTS-1);
		return;
	}

	if(m_pClient->m_Snap.m_SpecInfo.m_SpecMode == SpecMode && (SpecMode != SPEC_PLAYER || m_pClient->m_Snap.m_SpecInfo.m_SpectatorID == SpectatorID))
		return;

	CNetMsg_Cl_SetSpectatorMode Msg;
	Msg.m_SpecMode = SpecMode;
	Msg.m_SpectatorID = SpectatorID;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}
