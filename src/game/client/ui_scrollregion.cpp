/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <base/vmath.h>

#include <engine/client.h>
#include <engine/keys.h>

#include "ui_scrollregion.h"

CScrollRegion::CScrollRegion()
{
	m_ScrollY = 0;
	m_ContentH = 0;
	m_AnimTime = 0;
	m_AnimInitScrollY = 0;
	m_AnimTargetScrollY = 0;
	m_RequestScrollY = -1;
	m_ContentScrollOff = vec2(0,0);
	m_Params = CScrollRegionParams();
}

void CScrollRegion::Begin(CUIRect *pClipRect, vec2 *pOutOffset, CScrollRegionParams *pParams)
{
}

void CScrollRegion::End()
{
}

void CScrollRegion::AddRect(const CUIRect &Rect)
{
}

void CScrollRegion::ScrollHere(int Option)
{
}

bool CScrollRegion::IsRectClipped(const CUIRect &Rect) const
{
	return false;
}

bool CScrollRegion::IsScrollbarShown() const
{
	return false;
}

bool CScrollRegion::IsAnimating() const
{
	return false;
}
