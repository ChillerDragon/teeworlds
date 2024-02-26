/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/map.h>
#include <engine/storage.h>
#include <game/client/component.h>
#include <game/mapitems.h>

#include "mapimages.h"

CMapImages::CMapImages()
{
	m_Info[MAP_TYPE_GAME].m_Count = 0;
	m_Info[MAP_TYPE_MENU].m_Count = 0;

	m_EasterIsLoaded = false;
}

void CMapImages::LoadMapImages(IMap *pMap, class CLayers *pLayers, int MapType)
{
}

void CMapImages::OnMapLoad()
{
}

void CMapImages::OnMenuMapLoad(IMap *pMap)
{
}

int CMapImages::Num() const
{
	return 0;
}
