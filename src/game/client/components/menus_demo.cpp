/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/math.h>

#include <engine/demo.h>
#include <engine/keys.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include <game/client/render.h>
#include <game/client/gameclient.h>

#include <game/client/ui.h>

#include <generated/client_data.h>

#include "maplayers.h"
#include "menus.h"

CMenus::CColumn CMenus::ms_aDemoCols[] = { // Localize("Name"); Localize("Length"); Localize("Date"); - these strings are localized within CLocConstString
	{COL_DEMO_NAME,		CMenus::SORT_DEMONAME, "Name", 0, 100.0f, 0, {0}, {0}, TEXTALIGN_CENTER},
	{COL_DEMO_LENGTH,	CMenus::SORT_LENGTH, "Length", 1, 80.0f, 0, {0}, {0}, TEXTALIGN_CENTER},
	{COL_DEMO_DATE,		CMenus::SORT_DATE, "Date", 1, 170.0f, 0, {0}, {0}, TEXTALIGN_CENTER},
};

void CMenus::RenderDemoPlayer(CUIRect MainView)
{
}

void CMenus::DemolistPopulate()
{
}

void CMenus::DemolistOnUpdate(bool Reset)
{
}

bool CMenus::FetchHeader(CDemoItem *pItem)
{
	return false;
}

void CMenus::RenderDemoList(CUIRect MainView)
{
}

void CMenus::PopupConfirmDeleteDemo()
{
}

float CMenus::RenderDemoDetails(CUIRect View)
{
	return 0.0f;
}

void CMenus::Con_Play(IConsole::IResult *pResult, void *pUserData)
{
}
