/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <engine/config.h>
#include <engine/editor.h>
#include <engine/engine.h>
#include <engine/contacts.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <game/version.h>
#include <generated/protocol.h>

#include <generated/client_data.h>
#include <game/client/components/binds.h>
#include <game/client/components/camera.h>
#include <game/client/components/console.h>
#include <game/client/components/sounds.h>
#include <game/client/gameclient.h>
#include <game/client/lineinput.h>
#include <mastersrv/mastersrv.h>

#include "maplayers.h"
#include "countryflags.h"
#include "menus.h"
#include "skins.h"

CMenus::CMenus()
{
}

void CMenus::DoIcon(int ImageId, int SpriteId, const CUIRect *pRect, const vec4 *pColor)
{
}

bool CMenus::DoButton_Toggle(const void *pID, bool Checked, const CUIRect *pRect, bool Active)
{
	return false;
}

bool CMenus::DoButton_Menu(CButtonContainer *pBC, const char *pText, bool Checked, const CUIRect *pRect, const char *pImageName, int Corners, float Rounding, float FontFactor, vec4 ColorHot, bool TextFade)
{
	return false;
}

void CMenus::DoButton_KeySelect(CButtonContainer *pBC, const char *pText, const CUIRect *pRect)
{
}

bool CMenus::DoButton_MenuTabTop(CButtonContainer *pBC, const char *pText, bool Checked, const CUIRect *pRect, float Alpha, float FontAlpha, int Corners, float Rounding, float FontFactor)
{
	return false;
}

bool CMenus::DoButton_GridHeader(const void *pID, const char *pText, bool Checked, int Align, const CUIRect *pRect, int Corners)
{
	return false;
}

bool CMenus::DoButton_CheckBox(const void *pID, const char *pText, bool Checked, const CUIRect *pRect, bool Locked)
{
	return false;
}

bool CMenus::DoButton_SpriteID(CButtonContainer *pBC, int ImageID, int SpriteID, bool Checked, const CUIRect *pRect, int Corners, float Rounding, bool Fade)
{
	return false;
}

float CMenus::DoIndependentDropdownMenu(void *pID, const CUIRect *pRect, const char *pStr, float HeaderHeight, FDropdownCallback pfnCallback, bool *pActive)
{
	return 0.0f;
}

void CMenus::DoInfoBox(const CUIRect *pRect, const char *pLabel, const char *pValue)
{
}

void CMenus::DoJoystickBar(const CUIRect *pRect, float Current, float Tolerance, bool Active)
{
}

int CMenus::DoKeyReader(CButtonContainer *pBC, const CUIRect *pRect, int Key, int Modifier, int* pNewModifier)
{
	return 0;
}

void CMenus::RenderMenubar(CUIRect Rect)
{
}

void CMenus::InitLoading(int TotalWorkAmount)
{
}

void CMenus::RenderLoading(int WorkedAmount)
{
}

void CMenus::RenderNews(CUIRect MainView)
{
}

void CMenus::RenderBackButton(CUIRect MainView)
{
}

int CMenus::MenuImageScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0;
}

const CMenus::CMenuImage *CMenus::FindMenuImage(const char *pName)
{
	return 0;
}

void CMenus::UpdatedFilteredVideoModes()
{
}

void CMenus::UpdateVideoModeSettings()
{
}

int CMenus::GetInitAmount() const
{
	return 0;
}

void CMenus::OnInit()
{
}

void CMenus::PopupMessage(const char *pTitle, const char *pMessage, const char *pButtonLabel, int NextPopup, FPopupButtonCallback pfnButtonCallback)
{
}

void CMenus::PopupConfirm(const char *pTitle, const char *pMessage, const char *pConfirmButtonLabel, const char *pCancelButtonLabel,
	FPopupButtonCallback pfnConfirmButtonCallback, int ConfirmNextPopup, FPopupButtonCallback pfnCancelButtonCallback, int CancelNextPopup)
{
}

void CMenus::PopupCountry(int Selection, FPopupButtonCallback pfnOkButtonCallback)
{
}


void CMenus::RenderMenu(CUIRect Screen)
{
}


void CMenus::SetActive(bool Active)
{
}

void CMenus::OnReset()
{
}

bool CMenus::OnCursorMove(float x, float y, int CursorType)
{
	return true;
}

void CMenus::OnConsoleInit()
{
}

void CMenus::OnShutdown()
{
}

void CMenus::OnStateChange(int NewState, int OldState)
{
}

void CMenus::OnRender()
{
}

bool CMenus::CheckHotKey(int Key) const
{
	return false;
}

bool CMenus::IsBackgroundNeeded() const
{
	return false;
}

void CMenus::RenderBackground(float Time)
{
}

void CMenus::RenderBackgroundShadow(const CUIRect *pRect, bool TopToBottom, float Rounding)
{
}

void CMenus::ConchainUpdateMusicState(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
}

void CMenus::UpdateMusicState()
{
}

void CMenus::SetMenuPage(int NewPage)
{
}
