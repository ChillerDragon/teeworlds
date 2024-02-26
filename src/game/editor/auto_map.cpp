/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/console.h>
#include <engine/storage.h>

#include "auto_map.h"
#include "editor.h"


void CTilesetMapper::Load(const json_value &rElement)
{
}

const char* CTilesetMapper::GetRuleSetName(int Index) const
{
	return 0;
}

void CTilesetMapper::Proceed(CLayerTiles *pLayer, int ConfigID, RECTi Area)
{
}

void CDoodadsMapper::Load(const json_value &rElement)
{
}

const char* CDoodadsMapper::GetRuleSetName(int Index) const
{
	return 0;
}

void CDoodadsMapper::AnalyzeGameLayer()
{
}

void CDoodadsMapper::PlaceDoodads(CLayerTiles *pLayer, CRule *pRule, array<array<int> > *pPositions, int Amount, int LeftWall)
{
}

void CDoodadsMapper::Proceed(CLayerTiles *pLayer, int ConfigID, int Amount)
{
}
