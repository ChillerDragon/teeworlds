/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <engine/shared/datafile.h>
#include <engine/storage.h>
#include <engine/kernel.h>
#include <engine/map.h>
#include <engine/engine.h>
#include <engine/editor.h>
#include <engine/console.h>
#include <game/editor/editor.h>
#include <stdio.h>

array<CLayerGroup*> m_lGroups;
CEditorMap::CMapInfo m_MapInfo;


inline void IntsToStr(const int *pInts, int Num, char *pStr)
{
	while(Num)
	{
		pStr[0] = (((*pInts)>>24)&0xff)-128;
		pStr[1] = (((*pInts)>>16)&0xff)-128;
		pStr[2] = (((*pInts)>>8)&0xff)-128;
		pStr[3] = ((*pInts)&0xff)-128;
		pStr += 4;
		pInts++;
		Num--;
	}

	// null terminate
	pStr[-1] = 0;
}

int Load(class IStorage *pStorage, const char *pFileName, int StorageType)
{
	CDataFileReader DataFile;
	if(!DataFile.Open(pStorage, pFileName, StorageType))
		return 0;

	// Clean();

	// check version
	CMapItemVersion *pItem = (CMapItemVersion *)DataFile.FindItem(MAPITEMTYPE_VERSION, 0);
	if(!pItem)
	{
		return 0;
	}
	else if(pItem->m_Version != CMapItemVersion::CURRENT_VERSION)
	{
		return 0;
	}
	// load map info
	{
		CMapItemInfo *pItem = (CMapItemInfo *)DataFile.FindItem(MAPITEMTYPE_INFO, 0);
		if(pItem && pItem->m_Version == 1)
		{
			if(pItem->m_Author > -1)
				str_copy(m_MapInfo.m_aAuthor, (char *)DataFile.GetData(pItem->m_Author), sizeof(m_MapInfo.m_aAuthor));
			if(pItem->m_MapVersion > -1)
				str_copy(m_MapInfo.m_aVersion, (char *)DataFile.GetData(pItem->m_MapVersion), sizeof(m_MapInfo.m_aVersion));
			if(pItem->m_Credits > -1)
				str_copy(m_MapInfo.m_aCredits, (char *)DataFile.GetData(pItem->m_Credits), sizeof(m_MapInfo.m_aCredits));
			if(pItem->m_License > -1)
				str_copy(m_MapInfo.m_aLicense, (char *)DataFile.GetData(pItem->m_License), sizeof(m_MapInfo.m_aLicense));
		}
	}

	// load groups
	{
		int LayersStart, LayersNum;
		DataFile.GetType(MAPITEMTYPE_LAYER, &LayersStart, &LayersNum);

		int Start, Num;
		DataFile.GetType(MAPITEMTYPE_GROUP, &Start, &Num);
		for(int g = 0; g < Num; g++)
		{
			CMapItemGroup *pGItem = (CMapItemGroup *)DataFile.GetItem(Start+g, 0, 0);

			if(pGItem->m_Version < 1 || pGItem->m_Version > CMapItemGroup::CURRENT_VERSION)
				continue;

			// CLayerGroup *pGroup = NewGroup();
			// pGroup->m_ParallaxX = pGItem->m_ParallaxX;
			// pGroup->m_ParallaxY = pGItem->m_ParallaxY;
			// pGroup->m_OffsetX = pGItem->m_OffsetX;
			// pGroup->m_OffsetY = pGItem->m_OffsetY;

			// if(pGItem->m_Version >= 2)
			// {
			// 	pGroup->m_UseClipping = pGItem->m_UseClipping;
			// 	pGroup->m_ClipX = pGItem->m_ClipX;
			// 	pGroup->m_ClipY = pGItem->m_ClipY;
			// 	pGroup->m_ClipW = pGItem->m_ClipW;
			// 	pGroup->m_ClipH = pGItem->m_ClipH;
			// }

			// // load group name
			// if(pGItem->m_Version >= 3)
			// 	IntsToStr(pGItem->m_aName, sizeof(pGroup->m_aName)/sizeof(int), pGroup->m_aName);

			for(int l = 0; l < pGItem->m_NumLayers; l++)
			{
				CLayer *pLayer = 0;
				CMapItemLayer *pLayerItem = (CMapItemLayer *)DataFile.GetItem(LayersStart+pGItem->m_StartLayer+l, 0, 0);
				if(!pLayerItem)
					continue;

				if(pLayerItem->m_Type == LAYERTYPE_TILES)
				{
					CMapItemLayerTilemap *pTilemapItem = (CMapItemLayerTilemap *)pLayerItem;
					CLayerTiles *pTiles = 0;

					if(pTilemapItem->m_Flags&TILESLAYERFLAG_GAME)
					{
						// pTiles = new CLayerGame(pTilemapItem->m_Width, pTilemapItem->m_Height);
						// MakeGameLayer(pTiles);
						// MakeGameGroup(pGroup);
					}
					else
					{
						// pTiles = new CLayerTiles(pTilemapItem->m_Width, pTilemapItem->m_Height);
						// // pTiles->m_pEditor = m_pEditor;
						// pTiles->m_Color = pTilemapItem->m_Color;
						// pTiles->m_ColorEnv = pTilemapItem->m_ColorEnv;
						// pTiles->m_ColorEnvOffset = pTilemapItem->m_ColorEnvOffset;
					}

					pLayer = pTiles;

					// pGroup->AddLayer(pTiles);
					// void *pData = DataFile.GetData(pTilemapItem->m_Data);
					// pTiles->m_Image = pTilemapItem->m_Image;
					// pTiles->m_Game = pTilemapItem->m_Flags&TILESLAYERFLAG_GAME;

					// // load layer name
					// if(pTilemapItem->m_Version >= 3)
					// 	IntsToStr(pTilemapItem->m_aName, sizeof(pTiles->m_aName)/sizeof(int), pTiles->m_aName);

					// // get tile data
					// if(pTilemapItem->m_Version > 3)
					// 	pTiles->ExtractTiles((CTile *)pData);
					// else
					// 	mem_copy(pTiles->m_pTiles, pData, pTiles->m_Width*pTiles->m_Height*sizeof(CTile));


					// if(pTiles->m_Game && pTilemapItem->m_Version == MakeVersion(1, *pTilemapItem))
					// {
					// 	for(int i = 0; i < pTiles->m_Width*pTiles->m_Height; i++)
					// 	{
					// 		if(pTiles->m_pTiles[i].m_Index)
					// 			pTiles->m_pTiles[i].m_Index += ENTITY_OFFSET;
					// 	}
					// }

					DataFile.UnloadData(pTilemapItem->m_Data);
				}

				if(pLayer)
					pLayer->m_Flags = pLayerItem->m_Flags;
			}
		}
	}
	return 1;
}

int main(int argc, const char **argv)
{
	if(argc != 3)
	{
		dbg_msg("error", "usage: <input> <output>");
		return -1;
	}

	char aInFileName[1024];
	char aOutFileName[1024];
	IKernel *pKernel = IKernel::Create();
	IStorage *pStorage = CreateStorage("Teeworlds", IStorage::STORAGETYPE_BASIC, argc, argv);
	IEngine *pEngine = CreateEngine("Teeworlds");
	IEngineMap *pEngineMap = CreateEngineMap();

	{
		bool RegisterFail = false;

		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pEngine);

		RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IEngineMap*>(pEngineMap)); // register as both
		RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IMap*>(pEngineMap));

		RegisterFail = RegisterFail || !pKernel->RegisterInterface(pStorage);

		if(RegisterFail)
			return -1;
	}

	str_format(aInFileName, sizeof(aInFileName), "%s", argv[1]);
	str_format(aOutFileName, sizeof(aOutFileName), "%s", argv[2]);

	// if(!yeet->Load(aInFileName, IStorage::TYPE_ALL))
	// {
	// 	dbg_msg("error", "map '%s' not found", aInFileName);
	// 	return -1;
	// }

	return 0;
}
