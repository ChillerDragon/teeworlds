/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <algorithm>

#include <engine/keys.h>
#include <engine/input.h>
#include <engine/textrender.h>
#include <engine/graphics.h>

#include "lineinput.h"

IInput *CLineInput::s_pInput = 0;
ITextRender *CLineInput::s_pTextRender = 0;
IGraphics *CLineInput::s_pGraphics = 0;

CLineInput *CLineInput::s_pActiveInput = 0;
EInputPriority CLineInput::s_ActiveInputPriority = NONE;

vec2 CLineInput::s_CompositionWindowPosition = vec2(0, 0);
float CLineInput::s_CompositionLineHeight = 0.0f;

char CLineInput::s_aStars[128] = { '\0' };

void CLineInput::SetBuffer(char *pStr, int MaxSize, int MaxChars)
{
}

void CLineInput::Clear()
{
}

void CLineInput::Set(const char *pString)
{
}

void CLineInput::SetRange(const char *pString, int Begin, int End)
{
}

void CLineInput::Insert(const char *pString, int Begin)
{
}

void CLineInput::Append(const char *pString)
{
}

void CLineInput::UpdateStrData()
{
}

const char *CLineInput::GetDisplayedString()
{
	return 0;
}

void CLineInput::MoveCursor(EMoveDirection Direction, bool MoveWord, const char *pStr, int MaxSize, int *pCursorPos)
{
}

void CLineInput::SetCursorOffset(int Offset)
{
	m_SelectionStart = m_SelectionEnd = m_CursorPos = clamp(Offset, 0, m_Len);
}

void CLineInput::SetSelection(int Start, int End)
{
}

int CLineInput::OffsetFromActualToDisplay(int ActualOffset) const
{
	return 0;
}

int CLineInput::OffsetFromDisplayToActual(int DisplayOffset) const
{
	return 0;
}

bool CLineInput::ProcessInput(const IInput::CEvent &Event)
{
	return false;
}

void CLineInput::Render()
{
}

void CLineInput::RenderCandidates()
{
}

void CLineInput::DrawSelection(float HeightWeight, int Start, int End, vec4 Color)
{
}

void CLineInput::SetCompositionWindowPosition(vec2 Anchor, float LineHeight)
{
}

void CLineInput::Activate(EInputPriority Priority)
{
}

void CLineInput::Deactivate()
{
}

void CLineInput::OnActivate()
{
}

void CLineInput::OnDeactivate()
{
}
