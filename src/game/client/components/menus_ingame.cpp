/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>

#include <engine/config.h>
#include <engine/demo.h>
#include <engine/contacts.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/ui.h>

#include "menus.h"
#include "motd.h"
#include "voting.h"
#include "broadcast.h"

void CMenus::GetSwitchTeamInfo(CSwitchTeamInfo *pInfo)
{
}

void CMenus::RenderGame(CUIRect MainView)
{

}

void CMenus::RenderPlayers(CUIRect MainView)
{
}

void CMenus::RenderServerInfo(CUIRect MainView)
{
}

bool CMenus::RenderServerControlServer(CUIRect MainView)
{
	return false;
}

void CMenus::RenderServerControlKick(CUIRect MainView, bool FilterSpectators)
{
}

void CMenus::HandleCallvote(int Page, bool Force)
{
}

void CMenus::RenderServerControl(CUIRect MainView)
{
}
