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
static int s_aDifficultySpriteIds[] = {
	SPRITE_LEVEL_A_ON,
	SPRITE_LEVEL_B_ON,
	SPRITE_LEVEL_C_ON };

vec3 TextHighlightColor = vec3(0.4f, 0.4f, 1.0f);

// filters
CMenus::CBrowserFilter::CBrowserFilter(int Custom, const char* pName, IServerBrowser *pServerBrowser)
	: m_DeleteButtonContainer(true), m_UpButtonContainer(true), m_DownButtonContainer(true)
{
	m_Extended = false;
	m_Custom = Custom;
	str_copy(m_aName, pName, sizeof(m_aName));
	m_pServerBrowser = pServerBrowser;
	switch(m_Custom)
	{
	case CBrowserFilter::FILTER_STANDARD:
		m_Filter = m_pServerBrowser->AddFilter(&ms_FilterStandard);
		break;
	case CBrowserFilter::FILTER_RACE:
		m_Filter = m_pServerBrowser->AddFilter(&ms_FilterRace);
		break;
	case CBrowserFilter::FILTER_FAVORITES:
		m_Filter = m_pServerBrowser->AddFilter(&ms_FilterFavorites);
		break;
	default:
		m_Filter = m_pServerBrowser->AddFilter(&ms_FilterAll);
	}
}

void CMenus::CBrowserFilter::Reset()
{
	switch(m_Custom)
	{
	case CBrowserFilter::FILTER_STANDARD:
		SetFilter(&ms_FilterStandard);
		break;
	case CBrowserFilter::FILTER_RACE:
		SetFilter(&ms_FilterRace);
		break;
	case CBrowserFilter::FILTER_FAVORITES:
		SetFilter(&ms_FilterFavorites);
		break;
	default:
		SetFilter(&ms_FilterAll);
	}
}

void CMenus::CBrowserFilter::Switch()
{
	m_Extended ^= 1;
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
	m_Filter = Num;
}

void CMenus::CBrowserFilter::GetFilter(CServerFilterInfo *pFilterInfo) const
{
	m_pServerBrowser->GetFilter(m_Filter, pFilterInfo);
}

void CMenus::CBrowserFilter::SetFilter(const CServerFilterInfo *pFilterInfo)
{
	m_pServerBrowser->SetFilter(m_Filter, pFilterInfo);
}

void CMenus::LoadFilters()
{
	// read file data into buffer
	const char *pFilename = "ui_settings.json";
	IOHANDLE File = Storage()->OpenFile(pFilename, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
		return;
	int FileSize = (int)io_length(File);
	char *pFileData = (char *)mem_alloc(FileSize);
	io_read(File, pFileData, FileSize);
	io_close(File);

	// parse json data
	json_settings JsonSettings;
	mem_zero(&JsonSettings, sizeof(JsonSettings));
	char aError[256];
	json_value *pJsonData = json_parse_ex(&JsonSettings, pFileData, FileSize, aError);
	mem_free(pFileData);

	if(pJsonData == 0)
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "failed to load filters from '%s': %s", pFilename, aError);
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
		return;
	}

	// extract settings data
	const json_value &rSettingsEntry = (*pJsonData)["settings"];
	if(rSettingsEntry["sidebar_active"].type == json_integer)
		m_SidebarActive = rSettingsEntry["sidebar_active"].u.integer;
	if(rSettingsEntry["sidebar_tab"].type == json_integer)
		m_SidebarTab = clamp(int(rSettingsEntry["sidebar_tab"].u.integer), int(SIDEBAR_TAB_INFO), int(NUM_SIDEBAR_TABS-1));

	const int AllFilterIndex = CBrowserFilter::NUM_FILTERS - 2; // -2 because custom filters have index 0 but come last in the list
	if(rSettingsEntry["filters"].type == json_array)
	{
		for(unsigned i = 0; i < IServerBrowser::NUM_TYPES; ++i)
		{
			if(i < rSettingsEntry["filters"].u.array.length && rSettingsEntry["filters"][i].type == json_integer)
				m_aSelectedFilters[i] = rSettingsEntry["filters"][i].u.integer;
			else
				m_aSelectedFilters[i] = AllFilterIndex; // default to "all" if not set for all filters
		}
	}
	else
	{
		for(unsigned i = 0; i < IServerBrowser::NUM_TYPES; ++i)
			m_aSelectedFilters[i] = AllFilterIndex; // default to "all" if not set
	}

	// extract filter data
	const json_value &rFilterEntry = (*pJsonData)["filter"];
	for(unsigned i = 0; i < rFilterEntry.u.array.length; ++i)
	{
		char *pName = rFilterEntry[i].u.object.values[0].name;
		const json_value &rStart = *(rFilterEntry[i].u.object.values[0].value);
		if(rStart.type != json_object)
			continue;

		int Type = CBrowserFilter::FILTER_CUSTOM;
		if(rStart["type"].type == json_integer)
			Type = rStart["type"].u.integer;

		// filter setting
		CServerFilterInfo FilterInfo;
		for(int j = 0; j < CServerFilterInfo::MAX_GAMETYPES; ++j)
		{
			FilterInfo.m_aGametype[j][0] = 0;
			FilterInfo.m_aGametypeExclusive[j] = false;
		}
		const json_value &rSubStart = rStart["settings"];
		if(rSubStart.type == json_object)
		{
			if(rSubStart["filter_hash"].type == json_integer)
				FilterInfo.m_SortHash = rSubStart["filter_hash"].u.integer;

			const json_value &rGametypeEntry = rSubStart["filter_gametype"];
			if(rGametypeEntry.type == json_array) // legacy: all entries are inclusive
			{
				for(unsigned j = 0; j < rGametypeEntry.u.array.length && j < CServerFilterInfo::MAX_GAMETYPES; ++j)
				{
					if(rGametypeEntry[j].type == json_string)
					{
						str_copy(FilterInfo.m_aGametype[j], rGametypeEntry[j].u.string.ptr, sizeof(FilterInfo.m_aGametype[j]));
						FilterInfo.m_aGametypeExclusive[j] = false;
					}
				}
			}
			else if(rGametypeEntry.type == json_object)
			{
				for(unsigned j = 0; j < rGametypeEntry.u.object.length && j < CServerFilterInfo::MAX_GAMETYPES; ++j)
				{
					const json_value &rValue = *(rGametypeEntry.u.object.values[j].value);
					if(rValue.type == json_boolean)
					{
						str_copy(FilterInfo.m_aGametype[j], rGametypeEntry.u.object.values[j].name, sizeof(FilterInfo.m_aGametype[j]));
						FilterInfo.m_aGametypeExclusive[j] = rValue.u.boolean;
					}
				}
			}

			if(rSubStart["filter_ping"].type == json_integer)
				FilterInfo.m_Ping = rSubStart["filter_ping"].u.integer;
			if(rSubStart["filter_serverlevel"].type == json_integer)
				FilterInfo.m_ServerLevel = rSubStart["filter_serverlevel"].u.integer;
			if(rSubStart["filter_address"].type == json_string)
				str_copy(FilterInfo.m_aAddress, rSubStart["filter_address"].u.string.ptr, sizeof(FilterInfo.m_aAddress));
			if(rSubStart["filter_country"].type == json_integer)
				FilterInfo.m_Country = rSubStart["filter_country"].u.integer;
		}

		m_lFilters.add(CBrowserFilter(Type, pName, ServerBrowser()));

		if(Type == CBrowserFilter::FILTER_STANDARD) // make sure the pure filter is enabled in the Teeworlds-filter
			FilterInfo.m_SortHash |= IServerBrowser::FILTER_PURE;
		else if(Type == CBrowserFilter::FILTER_RACE) // make sure Race gametype is included in Race-filter
		{
			str_copy(FilterInfo.m_aGametype[0], "Race", sizeof(FilterInfo.m_aGametype[0]));
			FilterInfo.m_aGametypeExclusive[0] = false;
		}

		m_lFilters[i].SetFilter(&FilterInfo);
	}

	// clean up
	json_value_free(pJsonData);

	CBrowserFilter *pSelectedFilter = GetSelectedBrowserFilter();
	if(pSelectedFilter)
		pSelectedFilter->Switch();
}

void CMenus::SaveFilters()
{
	IOHANDLE File = Storage()->OpenFile("ui_settings.json", IOFLAG_WRITE, IStorage::TYPE_SAVE);
	if(!File)
		return;

	CJsonWriter Writer(File);

	Writer.BeginObject(); // root

	// settings
	Writer.WriteAttribute("settings");
	Writer.BeginObject();
	{
		Writer.WriteAttribute("sidebar_active");
		Writer.WriteIntValue(m_SidebarActive);

		Writer.WriteAttribute("sidebar_tab");
		Writer.WriteIntValue(m_SidebarTab);

		Writer.WriteAttribute("filters");
		Writer.BeginArray();
		for(int i = 0; i < IServerBrowser::NUM_TYPES; i++)
			Writer.WriteIntValue(m_aSelectedFilters[i]);
		Writer.EndArray();
	}
	Writer.EndObject();

	// filter
	Writer.WriteAttribute("filter");
	Writer.BeginArray();
	for(int i = 0; i < m_lFilters.size(); i++)
	{
		// part start
		Writer.BeginObject();
		Writer.WriteAttribute(m_lFilters[i].Name());
		Writer.BeginObject();
		{
			Writer.WriteAttribute("type");
			Writer.WriteIntValue(m_lFilters[i].Custom());

			// filter setting
			CServerFilterInfo FilterInfo;
			m_lFilters[i].GetFilter(&FilterInfo);

			Writer.WriteAttribute("settings");
			Writer.BeginObject();
			{
				Writer.WriteAttribute("filter_hash");
				Writer.WriteIntValue(FilterInfo.m_SortHash);

				Writer.WriteAttribute("filter_gametype");
				Writer.BeginObject();
				for(unsigned j = 0; j < CServerFilterInfo::MAX_GAMETYPES && FilterInfo.m_aGametype[j][0]; ++j)
				{
					Writer.WriteAttribute(FilterInfo.m_aGametype[j]);
					Writer.WriteBoolValue(FilterInfo.m_aGametypeExclusive[j]);
				}
				Writer.EndObject();

				Writer.WriteAttribute("filter_ping");
				Writer.WriteIntValue(FilterInfo.m_Ping);

				Writer.WriteAttribute("filter_serverlevel");
				Writer.WriteIntValue(FilterInfo.m_ServerLevel);

				Writer.WriteAttribute("filter_address");
				Writer.WriteStrValue(FilterInfo.m_aAddress);

				Writer.WriteAttribute("filter_country");
				Writer.WriteIntValue(FilterInfo.m_Country);
			}
			Writer.EndObject();
		}
		Writer.EndObject();
		Writer.EndObject();
	}
	Writer.EndArray();

	Writer.EndObject(); // end root
}

void CMenus::RemoveFilter(int FilterIndex)
{
	int Filter = m_lFilters[FilterIndex].Filter();
	ServerBrowser()->RemoveFilter(Filter);
	m_lFilters.remove_index(FilterIndex);

	// update filter indexes
	for(int i = 0; i < m_lFilters.size(); i++)
	{
		CBrowserFilter *pFilter = &m_lFilters[i];
		if(pFilter->Filter() > Filter)
			pFilter->SetFilterNum(pFilter->Filter()-1);
	}
}

void CMenus::MoveFilter(bool Up, int Filter)
{
	// move up
	CBrowserFilter Temp = m_lFilters[Filter];
	if(Up)
	{
		if(Filter > 0)
		{
			m_lFilters[Filter] = m_lFilters[Filter-1];
			m_lFilters[Filter-1] = Temp;
		}
	}
	else // move down
	{
		if(Filter < m_lFilters.size()-1)
		{
			m_lFilters[Filter] = m_lFilters[Filter+1];
			m_lFilters[Filter+1] = Temp;
		}
	}
}

void CMenus::InitDefaultFilters()
{
	int Filters = 0;
	for(int i = 0; i < m_lFilters.size(); i++)
		Filters |= 1 << m_lFilters[i].Custom();

	const bool UseDefaultFilters = Filters == 0;

	if((Filters & (1 << CBrowserFilter::FILTER_STANDARD)) == 0)
	{
		m_lFilters.add(CBrowserFilter(CBrowserFilter::FILTER_STANDARD, "Teeworlds", ServerBrowser()));
		for(int Pos = m_lFilters.size() - 1; Pos > 0; --Pos)
			MoveFilter(true, Pos);
	}

	if((Filters & (1 << CBrowserFilter::FILTER_RACE)) == 0)
	{
		m_lFilters.add(CBrowserFilter(CBrowserFilter::FILTER_RACE, Localize("Race"), ServerBrowser()));
		for(int Pos = m_lFilters.size() - 1; Pos > 1; --Pos)
			MoveFilter(true, Pos);
	}

	if((Filters & (1 << CBrowserFilter::FILTER_FAVORITES)) == 0)
	{
		m_lFilters.add(CBrowserFilter(CBrowserFilter::FILTER_FAVORITES, Localize("Favorites"), ServerBrowser()));
		for(int Pos = m_lFilters.size() - 1; Pos > 2; --Pos)
			MoveFilter(true, Pos);
	}

	if((Filters & (1 << CBrowserFilter::FILTER_ALL)) == 0)
	{
		m_lFilters.add(CBrowserFilter(CBrowserFilter::FILTER_ALL, Localize("All"), ServerBrowser()));
		for(int Pos = m_lFilters.size() - 1; Pos > 3; --Pos)
			MoveFilter(true, Pos);
	}

	// expand the all filter tab by default
	if(UseDefaultFilters)
	{
		const int AllFilterIndex = m_lFilters.size()-1;
		for(unsigned i = 0; i < IServerBrowser::NUM_TYPES; ++i)
			m_aSelectedFilters[i] = AllFilterIndex; // default to "all" if not set
		m_lFilters[AllFilterIndex].Switch();
	}
}

// 1 = browser entry click, 2 = server info click
int CMenus::DoBrowserEntry(const void *pID, CUIRect View, const CServerInfo *pEntry, const CBrowserFilter *pFilter, bool Selected, bool ShowServerInfo, CScrollRegion *pScroll)
{
	// logic
	int ReturnValue = 0;

	const bool Hovered = UI()->MouseHovered(&View);
	bool Highlighted = Hovered && (!pScroll || !pScroll->IsAnimating());

	if(UI()->CheckActiveItem(pID))
	{
		if(!UI()->MouseButton(0))
		{
			if(Hovered)
				ReturnValue = 1;
			UI()->SetActiveItem(0);
		}
	}
	if(UI()->HotItem() == pID)
	{
		if(UI()->MouseButton(0))
			UI()->SetActiveItem(pID);
	}

	if(Highlighted)
	{
		UI()->SetHotItem(pID);
		View.Draw(vec4(1.0f, 1.0f, 1.0f, 0.5f));
	}
	else if(Selected)
	{
		View.Draw(vec4(0.8f, 0.8f, 0.8f, 0.5f));
	}

	const float FontSize = 12.0f;
	const float TextAlpha = (pEntry->m_NumClients == pEntry->m_MaxClients) ? 0.5f : 1.0f;
	vec4 TextBaseColor = vec4(1.0f, 1.0f, 1.0f, TextAlpha);
	vec4 TextBaseOutlineColor = vec4(0.0, 0.0, 0.0, 0.3f);
	vec4 ServerInfoTextBaseColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	vec4 HighlightColor = vec4(TextHighlightColor.r, TextHighlightColor.g, TextHighlightColor.b, TextAlpha);
	if(Selected || Highlighted)
	{
		TextBaseColor = vec4(0.0f, 0.0f, 0.0f, TextAlpha);
		ServerInfoTextBaseColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		TextBaseOutlineColor = vec4(0.8f, 0.8f, 0.8f, 0.25f);
	}

	for(int c = 0; c < NUM_BROWSER_COLS; c++)
	{
		CUIRect Button = ms_aBrowserCols[c].m_Rect;
		char aTemp[64];
		Button.x = ms_aBrowserCols[c].m_Rect.x;
		Button.y = View.y;
		Button.h = ms_aBrowserCols[c].m_Rect.h;
		Button.w = ms_aBrowserCols[c].m_Rect.w;

		int ID = ms_aBrowserCols[c].m_ID;

		if(ID == COL_BROWSER_FLAG)
		{
			CUIRect Rect = Button;
			CUIRect Icon;

			Rect.VSplitLeft(Rect.h, &Icon, &Rect);
			if(pEntry->m_Flags&IServerBrowser::FLAG_PASSWORD)
			{
				Icon.Margin(2.0f, &Icon);
				DoIcon(IMAGE_BROWSEICONS, Selected ? SPRITE_BROWSE_LOCK_B : SPRITE_BROWSE_LOCK_A, &Icon);
				UI()->DoTooltip(&pEntry->m_Flags, &Icon, Localize("This server is protected by a password."));
			}

			Rect.VSplitLeft(Rect.h, &Icon, &Rect);
			Icon.Margin(2.0f, &Icon);
			DoIcon(IMAGE_LEVELICONS, s_aDifficultySpriteIds[pEntry->m_ServerLevel], &Icon);
			UI()->DoTooltip(&pEntry->m_ServerLevel, &Icon, s_aDifficultyLabels[pEntry->m_ServerLevel]);

			Rect.VSplitLeft(Rect.h, &Icon, &Rect);
			Icon.Margin(2.0f, &Icon);
			DoIcon(IMAGE_BROWSEICONS, pEntry->m_Favorite ? SPRITE_BROWSE_STAR_A : SPRITE_BROWSE_STAR_B, &Icon);
			if(UI()->DoButtonLogic(&pEntry->m_Favorite, &Icon))
			{
				if(!pEntry->m_Favorite)
					ServerBrowser()->AddFavorite(pEntry);
				else
					ServerBrowser()->RemoveFavorite(pEntry);
			}
			UI()->DoTooltip(&pEntry->m_Favorite, &Icon, pEntry->m_Favorite ? Localize("Click to remove server from favorites.") : Localize("Click to add server to favorites."));

			Rect.VSplitLeft(Rect.h, &Icon, &Rect);
			if(pEntry->m_FriendState != CContactInfo::CONTACT_NO)
			{
				Icon.Margin(2.0f, &Icon);
				Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BROWSEICONS].m_Id);
				Graphics()->QuadsBegin();
				Graphics()->SetColor(1.0f, 0.75f, 1.0f, 1.0f);
				RenderTools()->SelectSprite(Selected ? SPRITE_BROWSE_HEART_B : SPRITE_BROWSE_HEART_A);
				IGraphics::CQuadItem QuadItem(Icon.x, Icon.y, Icon.w, Icon.h);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
				Graphics()->QuadsEnd();
				UI()->DoTooltip(&pEntry->m_FriendState, &Icon, Localize("Friends are online on this server."));
			}
		}
		else if(ID == COL_BROWSER_NAME)
		{
			TextRender()->TextColor(TextBaseColor);
			TextRender()->TextSecondaryColor(TextBaseOutlineColor);
			Button.y += (Button.h - FontSize/CUI::ms_FontmodHeight)/2.0f;
			UI()->DoLabelHighlighted(&Button, pEntry->m_aName, (pEntry->m_QuickSearchHit&IServerBrowser::QUICK_SERVERNAME) ? Config()->m_BrFilterString : 0, FontSize, TextBaseColor, HighlightColor);
		}
		else if(ID == COL_BROWSER_MAP)
		{
			TextRender()->TextColor(TextBaseColor);
			Button.y += (Button.h - FontSize/CUI::ms_FontmodHeight)/2.0f;
			UI()->DoLabelHighlighted(&Button, pEntry->m_aMap, (pEntry->m_QuickSearchHit&IServerBrowser::QUICK_MAPNAME) ? Config()->m_BrFilterString : 0, FontSize, TextBaseColor, HighlightColor);
		}
		else if(ID == COL_BROWSER_PLAYERS)
		{
			TextRender()->TextColor(TextBaseColor);
			TextRender()->TextSecondaryColor(TextBaseOutlineColor);
			CServerFilterInfo FilterInfo;
			pFilter->GetFilter(&FilterInfo);

			int Num = (FilterInfo.m_SortHash&IServerBrowser::FILTER_SPECTATORS) ? pEntry->m_NumPlayers : pEntry->m_NumClients;
			int Max = (FilterInfo.m_SortHash&IServerBrowser::FILTER_SPECTATORS) ? pEntry->m_MaxPlayers : pEntry->m_MaxClients;
			if(FilterInfo.m_SortHash&IServerBrowser::FILTER_SPECTATORS)
			{
				int SpecNum = pEntry->m_NumClients - pEntry->m_NumPlayers;
				if(pEntry->m_MaxClients - pEntry->m_MaxPlayers < SpecNum)
					Max -= SpecNum;
			}
			if(FilterInfo.m_SortHash&IServerBrowser::FILTER_BOTS)
			{
				Num -= pEntry->m_NumBotPlayers;
				Max -= pEntry->m_NumBotPlayers;
				if(!(FilterInfo.m_SortHash&IServerBrowser::FILTER_SPECTATORS))
				{
					Num -= pEntry->m_NumBotSpectators;
					Max -= pEntry->m_NumBotSpectators;
				}

			}
			static float s_RenderOffset = 0.0f;
			if(s_RenderOffset == 0.0f)
				s_RenderOffset = TextRender()->TextWidth(FontSize, "0", -1);

			str_format(aTemp, sizeof(aTemp), "%d/%d", Num, Max);
			if(Config()->m_BrFilterString[0] && (pEntry->m_QuickSearchHit&IServerBrowser::QUICK_PLAYER))
				TextRender()->TextColor(TextHighlightColor.r, TextHighlightColor.g, TextHighlightColor.b, TextAlpha);
			Button.y += (Button.h - FontSize/CUI::ms_FontmodHeight)/2.0f;

			if(Num < 100)
				Button.x += s_RenderOffset;
			if(Num < 10)
				Button.x += s_RenderOffset;
			if(!Num)
				TextRender()->TextColor(CUI::ms_TransparentTextColor);
			UI()->DoLabel(&Button, aTemp, FontSize, TEXTALIGN_LEFT);
			Button.x += TextRender()->TextWidth(FontSize, aTemp, -1);
		}
		else if(ID == COL_BROWSER_PING)
		{
			const int Ping = pEntry->m_Latency;

			vec4 Color;
			if(Selected || Highlighted)
			{
				Color = TextBaseColor;
			}
			else
			{
				vec4 StartColor;
				vec4 EndColor;
				float MixVal;
				if(Ping <= 125)
				{
					StartColor = vec4(0.0f, 1.0f, 0.0f, TextAlpha);
					EndColor = vec4(1.0f, 1.0f, 0.0f, TextAlpha);

					MixVal = (Ping-50.0f)/75.0f;
				}
				else
				{
					StartColor = vec4(1.0f, 1.0f, 0.0f, TextAlpha);
					EndColor = vec4(1.0f, 0.0f, 0.0f, TextAlpha);

					MixVal = (Ping-125.0f)/75.0f;
				}
				Color = mix(StartColor, EndColor, MixVal);
			}

			str_format(aTemp, sizeof(aTemp), "%d", Ping);
			TextRender()->TextColor(Color);
			TextRender()->TextSecondaryColor(TextBaseOutlineColor);
			Button.y += (Button.h - FontSize/CUI::ms_FontmodHeight)/2.0f;
			Button.w -= 4.0f;
			UI()->DoLabel(&Button, aTemp, FontSize, TEXTALIGN_RIGHT);
		}
		else if(ID == COL_BROWSER_GAMETYPE)
		{
			// gametype icon
			CUIRect Icon;
			Button.VSplitLeft(Button.h, &Icon, &Button);
			Icon.y -= 0.5f;
			DoGameIcon(pEntry->m_aGameType, &Icon);

			// gametype text
			TextRender()->TextColor(TextBaseColor);
			TextRender()->TextSecondaryColor(TextBaseOutlineColor);
			Button.y += (Button.h - FontSize/CUI::ms_FontmodHeight)/2.0f;
			UI()->DoLabelHighlighted(&Button, pEntry->m_aGameType, (pEntry->m_QuickSearchHit&IServerBrowser::QUICK_GAMETYPE) ? Config()->m_BrFilterString : 0, FontSize, TextBaseColor, HighlightColor);
		}
	}

	// show server info
	if(ShowServerInfo)
	{
		View.HSplitTop(ms_aBrowserCols[0].m_Rect.h, 0, &View);

		if(ReturnValue && UI()->MouseHovered(&View))
			ReturnValue++;

		CUIRect Info, Scoreboard;
		View.VSplitLeft(160.0f, &Info, &Scoreboard);
		RenderDetailInfo(Info, pEntry, ServerInfoTextBaseColor, TextBaseOutlineColor);
		RenderDetailScoreboard(Scoreboard, pEntry, 4, ServerInfoTextBaseColor, TextBaseOutlineColor);
	}

	TextRender()->TextColor(CUI::ms_DefaultTextColor);
	TextRender()->TextSecondaryColor(CUI::ms_DefaultTextOutlineColor);

	return ReturnValue;
}

void CMenus::RenderFilterHeader(CUIRect View, int FilterIndex)
{
	CBrowserFilter *pFilter = &m_lFilters[FilterIndex];

	float ButtonHeight = 20.0f;
	float Spacing = 3.0f;
	bool Switch = false;

	View.Draw(vec4(0.0f, 0.0f, 0.0f, 0.25f));

	CUIRect Button, EditButtons;
	if(UI()->DoButtonLogic(pFilter, &View))
	{
		Switch = true; // switch later, to make sure we haven't clicked one of the filter buttons (edit...)
	}
	vec4 Color = UI()->MouseHovered(&View) ? vec4(1.0f, 1.0f, 1.0f, 1.0f) : vec4(0.6f, 0.6f, 0.6f, 1.0f);
	View.VSplitLeft(20.0f, &Button, &View);
	Button.Margin(2.0f, &Button);
	DoIcon(IMAGE_MENUICONS, pFilter->Extended() ? SPRITE_MENU_EXPANDED : SPRITE_MENU_COLLAPSED, &Button, &Color);

	// split buttons from label
	View.VSplitLeft(Spacing, 0, &View);
	View.VSplitRight((ButtonHeight+Spacing)*4.0f, &View, &EditButtons);

	View.VSplitLeft(20.0f, 0, &View); // little space
	UI()->DoLabel(&View, pFilter->Name(), ButtonHeight*CUI::ms_FontmodHeight*0.8f, TEXTALIGN_ML);

	View.VSplitRight(20.0f, &View, 0); // little space
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), Localize("%d servers, %d players"), pFilter->NumSortedServers(), pFilter->NumPlayers());
	UI()->DoLabel(&View, aBuf, ButtonHeight*CUI::ms_FontmodHeight*0.8f, TEXTALIGN_RIGHT);

	EditButtons.VSplitRight(ButtonHeight, &EditButtons, &Button);
	Button.Margin(2.0f, &Button);
	if(pFilter->Custom() == CBrowserFilter::FILTER_CUSTOM)
	{
		if(DoButton_SpriteID(&pFilter->m_DeleteButtonContainer, IMAGE_TOOLICONS, SPRITE_TOOL_X_A, false, &Button))
		{
			m_RemoveFilterIndex = FilterIndex;
			str_format(aBuf, sizeof(aBuf), Localize("Are you sure that you want to remove the filter '%s' from the server browser?"), pFilter->Name());
			PopupConfirm(Localize("Remove filter"), aBuf, Localize("Yes"), Localize("No"), &CMenus::PopupConfirmRemoveFilter);
		}
	}
	else
		DoIcon(IMAGE_TOOLICONS, SPRITE_TOOL_X_B, &Button);

	EditButtons.VSplitRight(Spacing, &EditButtons, 0);
	EditButtons.VSplitRight(ButtonHeight, &EditButtons, &Button);
	Button.Margin(2.0f, &Button);
	if(FilterIndex > 0)
	{
		if(DoButton_SpriteID(&pFilter->m_UpButtonContainer, IMAGE_TOOLICONS, SPRITE_TOOL_UP_A, false, &Button))
		{
			MoveFilter(true, FilterIndex);
			Switch = false;
		}
	}
	else
		DoIcon(IMAGE_TOOLICONS, SPRITE_TOOL_UP_B, &Button);

	EditButtons.VSplitRight(Spacing, &EditButtons, 0);
	EditButtons.VSplitRight(ButtonHeight, &EditButtons, &Button);
	Button.Margin(2.0f, &Button);
	if(FilterIndex < m_lFilters.size() - 1)
	{
		if(DoButton_SpriteID(&pFilter->m_DownButtonContainer, IMAGE_TOOLICONS, SPRITE_TOOL_DOWN_A, false, &Button))
		{
			MoveFilter(false, FilterIndex);
			Switch = false;
		}
	}
	else
		DoIcon(IMAGE_TOOLICONS, SPRITE_TOOL_DOWN_B, &Button);

	if(Switch)
	{
		pFilter->Switch();
		// retract the other filters
		if(pFilter->Extended())
		{
			for(int i = 0; i < m_lFilters.size(); ++i)
			{
				if(i != FilterIndex && m_lFilters[i].Extended())
					m_lFilters[i].Switch();
			}
		}
	}
}

void CMenus::PopupConfirmRemoveFilter()
{
	// remove filter
	if(m_RemoveFilterIndex)
	{
		RemoveFilter(m_RemoveFilterIndex);
	}
}

void CMenus::PopupConfirmCountryFilter()
{
}

static void FormatScore(char *pBuf, int BufSize, bool TimeScore, const CServerInfo::CClient *pClient)
{
	if(TimeScore)
		FormatTime(pBuf, BufSize, pClient->m_Score * 1000, 0);
	else
		str_format(pBuf, BufSize, "%d", pClient->m_Score);
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
