/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>
#include <algorithm>

#include <base/math.h>

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/map.h>
#include <engine/textrender.h>
#include <generated/client_data.h>
#include <game/layers.h>
#include "animstate.h"
#include "render.h"


void CRenderTools::Init(CConfig *pConfig, IGraphics *pGraphics)
{
	m_pConfig = pConfig;
	m_pGraphics = pGraphics;
}

void CRenderTools::SelectSprite(const CDataSprite *pSpr, int Flags, int sx, int sy)
{
}

void CRenderTools::SelectSprite(int Id, int Flags, int sx, int sy)
{
}

void CRenderTools::DrawSprite(float x, float y, float Size)
{
}

void CRenderTools::RenderCursor(float CenterX, float CenterY, float Size)
{
}

void CRenderTools::RenderTee(CAnimState *pAnim, const CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos)
{
}

void CRenderTools::RenderTeeHand(const CTeeRenderInfo *pInfo, vec2 CenterPos, vec2 Dir, float AngleOffset,
								 vec2 PostRotOffset)
{
}

void CRenderTools::MapScreenToWorld(float CenterX, float CenterY, float ParallaxX, float ParallaxY,
	float OffsetX, float OffsetY, float Aspect, float Zoom, float aPoints[4])
{
}

void CRenderTools::MapScreenToGroup(float CenterX, float CenterY, const CMapItemGroup *pGroup, float Zoom)
{
}
