/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/color.h>
#include <base/math.h>

#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/serverbrowser.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/external/json-parser/json.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/components/maplayers.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>

#include "binds.h"
#include "countryflags.h"
#include "menus.h"

CMenusKeyBinder::CMenusKeyBinder()
{
}

void CMenus::RenderHSLPicker(CUIRect MainView)
{
}

void CMenus::RenderSkinSelection(CUIRect MainView)
{
}

void CMenus::RenderSkinPartSelection(CUIRect MainView)
{
}

void CMenus::RenderSkinPartPalette(CUIRect MainView)
{
}

class CLanguage
{
public:
	CLanguage() {}
	CLanguage(const char *n, const char *f, int Code) : m_Name(n), m_FileName(f), m_CountryCode(Code) {}

	string m_Name;
	string m_FileName;
	int m_CountryCode;

	bool operator<(const CLanguage &Other) { return m_Name < Other.m_Name; }
};


int CMenus::ThemeScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0;
}

int CMenus::ThemeIconScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0; // no existing theme
}

void LoadLanguageIndexfile(IStorage *pStorage, IConsole *pConsole, sorted_array<CLanguage> *pLanguages)
{
}

void CMenus::RenderLanguageSelection(CUIRect MainView, bool Header)
{
}

void CMenus::RenderThemeSelection(CUIRect MainView, bool Header)
{
}

void CMenus::RenderSettingsGeneral(CUIRect MainView)
{
}

void CMenus::RenderSettingsTeeBasic(CUIRect MainView)
{
}

void CMenus::RenderSettingsTeeCustom(CUIRect MainView)
{
}

void CMenus::RenderSettingsPlayer(CUIRect MainView)
{
}

void CMenus::PopupConfirmDeleteSkin()
{
}

void CMenus::RenderSettingsControls(CUIRect MainView)
{
}

float CMenus::RenderSettingsControlsStats(CUIRect View)
{
	return 0.0f;
}

bool CMenus::DoResolutionList(CUIRect* pRect, CListBox* pListBox,
							  const sorted_array<CVideoMode>& lModes)
{
	return false;
}

void CMenus::RenderSettingsGraphics(CUIRect MainView)
{
}

void CMenus::RenderSettingsSound(CUIRect MainView)
{
}

void CMenus::ResetSettingsGeneral()
{
}

void CMenus::ResetSettingsControls()
{
}

void CMenus::ResetSettingsGraphics()
{
}

void CMenus::PopupConfirmPlayerCountry()
{
}

void CMenus::RenderSettings(CUIRect MainView)
{
}
