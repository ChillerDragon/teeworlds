/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <limits.h>

#include <engine/shared/config.h>

#include <generated/client_data.h>
#include <generated/protocol.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/localization.h>
#include <game/client/components/countryflags.h>
#include <game/client/components/motd.h>
#include <game/client/components/stats.h>

#include "menus.h"
#include "stats.h"
#include "scoreboard.h"
#include "stats.h"


CScoreboard::CScoreboard()
{
}

void CScoreboard::ConKeyScoreboard(IConsole::IResult *pResult, void *pUserData)
{
}

void CScoreboard::OnReset()
{
}

void CScoreboard::OnRelease()
{
}

void CScoreboard::OnConsoleInit()
{
}

void CScoreboard::RenderGoals(float x, float y, float w)
{
}

float CScoreboard::RenderSpectators(float x, float y, float w)
{
	return 0.0f;
}

float CScoreboard::RenderScoreboard(float x, float y, float w, int Team, const char *pTitle, int Align)
{
	return 0.0f;
}

void CScoreboard::RenderRecordingNotification(float x, float w)
{
}

void CScoreboard::RenderNetworkQuality(float x, float w)
{
}

void CScoreboard::OnRender()
{
}

bool CScoreboard::IsActive() const
{
	return false;
}

const char *CScoreboard::GetClanName(int Team)
{
	return 0;
}
