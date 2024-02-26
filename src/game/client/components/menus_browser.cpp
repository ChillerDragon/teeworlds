/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <algorithm> // sort  TODO: remove this

#include <engine/external/json-parser/json.h>

#include <engine/config.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <engine/shared/jsonwriter.h>
#include <engine/client/contacts.h>

#include <generated/client_data.h>
#include <generated/protocol.h>

#include <game/version.h>
#include <game/client/render.h>
#include <game/client/ui.h>
#include <game/client/components/countryflags.h>

#include "menus.h"

CMenus::CColumn CMenus::ms_aBrowserCols[] = {  // Localize("Server"); Localize("Type"); Localize("Map"); Localize("Players"); Localize("Ping"); - these strings are localized within CLocConstString
	{COL_BROWSER_FLAG,		-1,									" ",		-1, 4*16.0f+3*2.0f, 0, {0}, {0}, TEXTALIGN_CENTER},
	{COL_BROWSER_NAME,		IServerBrowser::SORT_NAME,			"Server",	0, 310.0f, 0, {0}, {0}, TEXTALIGN_CENTER},
	{COL_BROWSER_GAMETYPE,	IServerBrowser::SORT_GAMETYPE,		"Type",		1, 70.0f,  0, {0}, {0}, TEXTALIGN_CENTER},
	{COL_BROWSER_MAP,		IServerBrowser::SORT_MAP,			"Map",		1, 100.0f, 0, {0}, {0}, TEXTALIGN_CENTER},
	{COL_BROWSER_PLAYERS,	IServerBrowser::SORT_NUMPLAYERS,	"Players",	1, 50.0f,  0, {0}, {0}, TEXTALIGN_CENTER},
	{COL_BROWSER_PING,		IServerBrowser::SORT_PING,			"Ping",		1, 40.0f,  0, {0}, {0}, TEXTALIGN_CENTER},
};

CServerFilterInfo CMenus::CBrowserFilter::ms_FilterStandard = {IServerBrowser::FILTER_COMPAT_VERSION|IServerBrowser::FILTER_PURE|IServerBrowser::FILTER_PURE_MAP, 999, -1, 0, {{0}}, {0}, {0}};
CServerFilterInfo CMenus::CBrowserFilter::ms_FilterRace = {IServerBrowser::FILTER_COMPAT_VERSION, 999, -1, 0, {{"Race"}}, {false}, {0}};
CServerFilterInfo CMenus::CBrowserFilter::ms_FilterFavorites = {IServerBrowser::FILTER_COMPAT_VERSION|IServerBrowser::FILTER_FAVORITE, 999, -1, 0, {{0}}, {0}, {0}};
CServerFilterInfo CMenus::CBrowserFilter::ms_FilterAll = {IServerBrowser::FILTER_COMPAT_VERSION, 999, -1, 0, {{0}}, {0}, {0}};

static CLocConstString s_aDifficultyLabels[] = {
	"Casual",
	"Normal",
	"Competitive" };

vec3 TextHighlightColor = vec3(0.4f, 0.4f, 1.0f);

// filters
CMenus::CBrowserFilter::CBrowserFilter(int Custom, const char* pName, IServerBrowser *pServerBrowser)
	: m_DeleteButtonContainer(true), m_UpButtonContainer(true), m_DownButtonContainer(true)
{
}

void CMenus::CBrowserFilter::Reset()
{
}

void CMenus::CBrowserFilter::Switch()
{
}

bool CMenus::CBrowserFilter::Extended() const
{
	return m_Extended;
}

int CMenus::CBrowserFilter::Custom() const
{
	return m_Custom;
}

int CMenus::CBrowserFilter::Filter() const
{
	return m_Filter;
}

const char* CMenus::CBrowserFilter::Name() const
{
	return m_aName;
}

const void *CMenus::CBrowserFilter::ID(int Index) const
{
	return m_pServerBrowser->GetID(m_Filter, Index);
}

int CMenus::CBrowserFilter::NumSortedServers() const
{
	return m_pServerBrowser->NumSortedServers(m_Filter);
}

int CMenus::CBrowserFilter::NumPlayers() const
{
	return m_pServerBrowser->NumSortedPlayers(m_Filter);
}

const CServerInfo* CMenus::CBrowserFilter::SortedGet(int Index) const
{
	if(Index < 0 || Index >= m_pServerBrowser->NumSortedServers(m_Filter))
		return 0;
	return m_pServerBrowser->SortedGet(m_Filter, Index);
}

void CMenus::CBrowserFilter::SetFilterNum(int Num)
{
}

void CMenus::CBrowserFilter::GetFilter(CServerFilterInfo *pFilterInfo) const
{
}

void CMenus::CBrowserFilter::SetFilter(const CServerFilterInfo *pFilterInfo)
{
}

void CMenus::LoadFilters()
{
}

void CMenus::SaveFilters()
{
}

void CMenus::RemoveFilter(int FilterIndex)
{
}

void CMenus::MoveFilter(bool Up, int Filter)
{
}

void CMenus::InitDefaultFilters()
{
}

// 1 = browser entry click, 2 = server info click
int CMenus::DoBrowserEntry(const void *pID, CUIRect View, const CServerInfo *pEntry, const CBrowserFilter *pFilter, bool Selected, bool ShowServerInfo, CScrollRegion *pScroll)
{
	return 0;
}

void CMenus::RenderFilterHeader(CUIRect View, int FilterIndex)
{
}

void CMenus::PopupConfirmRemoveFilter()
{
}

void CMenus::PopupConfirmCountryFilter()
{
}

void CMenus::RenderServerbrowserServerList(CUIRect View)
{
}

void CMenus::RenderServerbrowserSidebar(CUIRect View)
{
}

void CMenus::RenderServerbrowserFriendTab(CUIRect View)
{
}

void CMenus::PopupConfirmRemoveFriend()
{
}

void CMenus::RenderServerbrowserFilterTab(CUIRect View)
{
}

void CMenus::RenderServerbrowserInfoTab(CUIRect View)
{
}

void CMenus::RenderDetailInfo(CUIRect View, const CServerInfo *pInfo, const vec4 &TextColor, const vec4 &TextOutlineColor)
{
}

void CMenus::RenderDetailScoreboard(CUIRect View, const CServerInfo *pInfo, int RowCount, const vec4 &TextColor, const vec4 &TextOutlineColor)
{
}

void CMenus::RenderServerbrowserServerDetail(CUIRect View, const CServerInfo *pInfo)
{
}

void CMenus::FriendlistOnUpdate()
{
}

void CMenus::RenderServerbrowserBottomBox(CUIRect MainView)
{
}

void CMenus::DoGameIcon(const char *pName, const CUIRect *pRect)
{
}

int CMenus::GameIconScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0;
}

void CMenus::RenderServerbrowser(CUIRect MainView)
{
}

void CMenus::UpdateServerBrowserAddress()
{
}

const char *CMenus::GetServerBrowserAddress()
{
	return 0;
}

void CMenus::SetServerBrowserAddress(const char *pAddress)
{
}

void CMenus::ServerBrowserFilterOnUpdate()
{
}

void CMenus::ServerBrowserSortingOnUpdate()
{
}

void CMenus::ConchainConnect(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
}

void CMenus::ConchainFriendlistUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
}

void CMenus::ConchainServerbrowserUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
}

void CMenus::ConchainServerbrowserSortingUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
}
