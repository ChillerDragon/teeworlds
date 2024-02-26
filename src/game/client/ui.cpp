/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>

#include <engine/shared/config.h>

#include <engine/client.h>
#include <engine/graphics.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/textrender.h>

#include "ui.h"

/********************************************************
 UI
*********************************************************/

const float CUI::ms_ListheaderHeight = 17.0f;
const float CUI::ms_FontmodHeight = 0.8f;

const vec4 CUI::ms_DefaultTextColor(1.0f, 1.0f, 1.0f, 1.0f);
const vec4 CUI::ms_DefaultTextOutlineColor(0.0f, 0.0f, 0.0f, 0.3f);
const vec4 CUI::ms_HighlightTextColor(0.0f, 0.0f, 0.0f, 1.0f);
const vec4 CUI::ms_HighlightTextOutlineColor(1.0f, 1.0f, 1.0f, 0.25f);
const vec4 CUI::ms_TransparentTextColor(1.0f, 1.0f, 1.0f, 0.5f);

CUI *CUIElementBase::s_pUI = 0;

IClient *CUIElementBase::Client() const { return s_pUI->Client(); }
CConfig *CUIElementBase::Config() const { return s_pUI->Config(); }
IGraphics *CUIElementBase::Graphics() const { return s_pUI->Graphics(); }
IInput *CUIElementBase::Input() const { return s_pUI->Input(); }
ITextRender *CUIElementBase::TextRender() const { return s_pUI->TextRender(); }

float CButtonContainer::GetFade(bool Checked, float Seconds)
{
	if(UI()->HotItem() == this || Checked)
	{
		m_FadeStartTime = Client()->LocalTime();
		return 1.0f;
	}

	return maximum(0.0f, m_FadeStartTime -  Client()->LocalTime() + Seconds)/Seconds;
}


CUI::CUI()
{
	m_pHotItem = 0;
	m_pActiveItem = 0;
	m_pLastActiveItem = 0;
	m_pBecommingHotItem = 0;

	m_MouseX = 0;
	m_MouseY = 0;
	m_MouseWorldX = 0;
	m_MouseWorldY = 0;
	m_MouseButtons = 0;
	m_LastMouseButtons = 0;
	m_Enabled = true;

	m_HotkeysPressed = 0;

	m_Screen.x = 0;
	m_Screen.y = 0;

	m_NumClips = 0;

	m_pActiveTooltip = 0;
	m_aTooltipText[0] = '\0';

	m_NumPopupMenus = 0;
}

void CUI::Init(class IKernel *pKernel)
{
	m_pClient = pKernel->RequestInterface<IClient>();
	m_pConfig = pKernel->RequestInterface<IConfigManager>()->Values();
	m_pGraphics = pKernel->RequestInterface<IGraphics>();
	m_pInput = pKernel->RequestInterface<IInput>();
	m_pTextRender = pKernel->RequestInterface<ITextRender>();
	CUIRect::Init(m_pGraphics);
	CLineInput::Init(m_pInput, m_pTextRender, m_pGraphics);
	CUIElementBase::Init(this);
}

void CUI::Update(float MouseX, float MouseY, float MouseWorldX, float MouseWorldY)
{
	unsigned MouseButtons = 0;
	if(Enabled())
	{
		if(Input()->KeyIsPressed(KEY_MOUSE_1)) MouseButtons |= 1;
		if(Input()->KeyIsPressed(KEY_MOUSE_2)) MouseButtons |= 2;
		if(Input()->KeyIsPressed(KEY_MOUSE_3)) MouseButtons |= 4;
	}

	m_MouseX = MouseX;
	m_MouseY = MouseY;
	m_MouseWorldX = MouseWorldX;
	m_MouseWorldY = MouseWorldY;
	m_LastMouseButtons = m_MouseButtons;
	m_MouseButtons = MouseButtons;
	m_pHotItem = m_pBecommingHotItem;
	if(m_pActiveItem)
		m_pHotItem = m_pActiveItem;
	m_pBecommingHotItem = 0;

	if(Enabled())
	{
		CLineInput *pActiveInput = CLineInput::GetActiveInput();
		if(pActiveInput && m_pLastActiveItem && pActiveInput != m_pLastActiveItem)
			pActiveInput->Deactivate();
	}
}

bool CUI::KeyPress(int Key) const
{
	return Enabled() && Input()->KeyPress(Key);
}

bool CUI::KeyIsPressed(int Key) const
{
	return Enabled() && Input()->KeyIsPressed(Key);
}

bool CUI::ConsumeHotkey(unsigned Hotkey)
{
	return false;
}

bool CUI::OnInput(const IInput::CEvent &e)
{
	return false;
}

void CUI::ConvertCursorMove(float *pX, float *pY, int CursorType) const
{
}

const CUIRect *CUI::Screen()
{
	m_Screen.h = 600;
	m_Screen.w = Graphics()->ScreenAspect() * m_Screen.h;
	return &m_Screen;
}

float CUI::PixelSize()
{
	return Screen()->w/Graphics()->ScreenWidth();
}

void CUI::MapScreen()
{
}

void CUI::ClipEnable(const CUIRect *pRect)
{
}

void CUI::ClipDisable()
{
}

const CUIRect *CUI::ClipArea() const
{
	return &m_aClips[m_NumClips - 1];
}

void CUI::UpdateClipping()
{
}

bool CUI::DoButtonLogic(const void *pID, const CUIRect *pRect, int Button)
{
	// logic
	bool Clicked = false;
	static int s_LastButton = -1;
	const bool Hovered = MouseHovered(pRect);

	if(CheckActiveItem(pID))
	{
		if(s_LastButton == Button && !MouseButton(s_LastButton))
		{
			if(Hovered)
				Clicked = true;
			SetActiveItem(0);
			s_LastButton = -1;
		}
	}
	else if(HotItem() == pID)
	{
		if(MouseButton(Button))
		{
			SetActiveItem(pID);
			s_LastButton = Button;
		}
	}

	if(Hovered && !MouseButton(Button))
		SetHotItem(pID);

	return Clicked;
}

bool CUI::DoPickerLogic(const void *pID, const CUIRect *pRect, float *pX, float *pY)
{
	return true;
}

void CUI::ApplyCursorAlign(class CTextCursor *pCursor, const CUIRect *pRect, int Align)
{
}

void CUI::DoLabel(const CUIRect *pRect, const char *pText, float FontSize, int Align, float LineWidth, bool MultiLine)
{
}

void CUI::DoLabelHighlighted(const CUIRect *pRect, const char *pText, const char *pHighlighted, float FontSize, const vec4 &TextColor, const vec4 &HighlightColor, int Align)
{
}

void CUI::DoLabelSelected(const CUIRect *pRect, const char *pText, bool Selected, float FontSize, int Align)
{
}

bool CUI::DoEditBox(CLineInput *pLineInput, const CUIRect *pRect, float FontSize, int Corners, const IButtonColorFunction *pColorFunction)
{
	return false;
}

void CUI::DoEditBoxOption(CLineInput *pLineInput, const CUIRect *pRect, const char *pStr, float VSplitVal)
{
}

float CUI::DoScrollbarV(const void *pID, const CUIRect *pRect, float Current)
{
	return 0.0f;
}

float CUI::DoScrollbarH(const void *pID, const CUIRect *pRect, float Current)
{
	return 0.0f;
}

void CUI::DoScrollbarOption(const void *pID, int *pOption, const CUIRect *pRect, const char *pStr, int Min, int Max, const IScrollbarScale *pScale, bool Infinite)
{
}

void CUI::DoScrollbarOptionLabeled(const void *pID, int *pOption, const CUIRect *pRect, const char *pStr, const char* aLabels[], int NumLabels, const IScrollbarScale *pScale)
{
}

float CUI::DrawClientID(float FontSize, vec2 CursorPosition, int ID, const vec4& BgColor, const vec4& TextColor)
{
	return 0.0f;
}

void CUI::DoTooltip(const void *pID, const CUIRect *pRect, const char *pText)
{
}

void CUI::RenderTooltip()
{
}

void CUI::DoPopupMenu(int X, int Y, int Width, int Height, void *pContext, bool (*pfnFunc)(void *pContext, CUIRect View), int Corners)
{
}

void CUI::RenderPopupMenus()
{
}

float CUI::GetClientIDRectWidth(float FontSize)
{
	if(!m_pConfig->m_ClShowUserId)
		return 0;
	return 1.4f * FontSize + 0.2f * FontSize;
}

float CUI::GetListHeaderHeight() const
{
	return ms_ListheaderHeight + (m_pConfig->m_UiWideview ? 3.0f : 0.0f);
}

float CUI::GetListHeaderHeightFactor() const
{
	return 1.0f + (m_pConfig->m_UiWideview ? (3.0f/ms_ListheaderHeight) : 0.0f);
}
