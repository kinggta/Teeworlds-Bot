/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <engine/client.h>
#include <engine/storage.h>
#include <engine/shared/datafile.h>
#include <game/gamecore.h>

#include "layers.h"

CLayers::CLayers()
{
	m_GroupsNum = 0;
	m_GroupsStart = 0;
	m_LayersNum = 0;
	m_LayersStart = 0;
	m_pGameGroup = 0;
	m_pGameLayer = 0;
	m_pMap = 0;

	m_pMapGroups.clear();
	m_pMapLayers.clear();
}

void CLayers::Init(class IKernel *pKernel)
{
	m_pMap = pKernel->RequestInterface<IMap>();
	m_pMap->GetType(MAPITEMTYPE_GROUP, &m_GroupsStart, &m_GroupsNum);
	m_pMap->GetType(MAPITEMTYPE_LAYER, &m_LayersStart, &m_LayersNum);

	m_pMapGroups.clear();
	m_pMapLayers.clear();

	for(int g = 0; g < NumGroups(); g++)
	{
		char aGroupName[128];
		mem_zero(&aGroupName, sizeof(aGroupName));
		CMapItemGroup *pGroup = GetGroup(g);

		IntsToStr(pGroup->m_aName, sizeof(aGroupName)/sizeof(int), aGroupName);
		if(pGroup->m_Version < 3 || aGroupName[0] == 0)
			str_format(aGroupName, sizeof(aGroupName), "#%i Group", g);

		for(int l = 0; l < pGroup->m_NumLayers; l++)
		{
			char aLayerName[128];
			mem_zero(&aLayerName, sizeof(aLayerName));
			CMapItemLayer *pLayer = GetLayer(pGroup->m_StartLayer+l);

			if(pLayer->m_Type == LAYERTYPE_TILES)
			{
				CMapItemLayerTilemap *pTilemap = reinterpret_cast<CMapItemLayerTilemap *>(pLayer);
				IntsToStr(pTilemap->m_aName, sizeof(aLayerName)/sizeof(int), aLayerName);

				if(pTilemap->m_Version < 3 || aLayerName[0] == 0)
					str_copy(aLayerName, "Tiles", sizeof(aLayerName));

				if(pTilemap->m_Flags&TILESLAYERFLAG_GAME)
				{
					m_pGameLayer = pTilemap;
					m_pGameGroup = pGroup;

					// make sure the game group has standard settings
					m_pGameGroup->m_OffsetX = 0;
					m_pGameGroup->m_OffsetY = 0;
					m_pGameGroup->m_ParallaxX = 100;
					m_pGameGroup->m_ParallaxY = 100;

					if(m_pGameGroup->m_Version >= 2)
					{
						m_pGameGroup->m_UseClipping = 0;
						m_pGameGroup->m_ClipX = 0;
						m_pGameGroup->m_ClipY = 0;
						m_pGameGroup->m_ClipW = 0;
						m_pGameGroup->m_ClipH = 0;
					}

					str_copy(aLayerName, "Game", sizeof(aLayerName));
					str_copy(aGroupName, "Game", sizeof(aGroupName));
				}
			}
			else if(pLayer->m_Type == LAYERTYPE_QUADS)
			{
				CMapItemLayerQuads *pQuads = reinterpret_cast<CMapItemLayerQuads *>(pLayer);
				IntsToStr(pQuads->m_aName, sizeof(aLayerName)/sizeof(int), aLayerName);

				if(pQuads->m_Version < 2 || aLayerName[0] == 0)
					str_copy(aLayerName, "Quads", sizeof(aLayerName));
			}

			m_pMapLayers.add(new CMapLayerEx(true, aLayerName));
		}

		m_pMapGroups.add(new CMapGroupEx(true, aGroupName));
	}
}

CMapItemGroup *CLayers::GetGroup(int Index)
{
	return static_cast<CMapItemGroup *>(m_pMap->GetItem(m_GroupsStart+Index, 0, 0));
}

CMapItemLayer *CLayers::GetLayer(int Index)
{
	return static_cast<CMapItemLayer *>(m_pMap->GetItem(m_LayersStart+Index, 0, 0));
}


CMapGroupEx *CLayers::GetGroupEx(int Index)
{
	return m_pMapGroups[Index];
}

CMapLayerEx *CLayers::GetLayerEx(int Index)
{
	return m_pMapLayers[Index];
}
