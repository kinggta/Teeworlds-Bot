/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_LAYERS_H
#define GAME_LAYERS_H

#include <base/tl/array.h>
#include <engine/map.h>
#include <game/mapitems.h>

struct CMapGroupEx
{
	CMapGroupEx(bool Visible, char *pName) : m_Visible(Visible)
	{m_ButtonID = -1; str_copy(m_aName, pName, sizeof(m_aName));}
	int m_ButtonID;
	char m_aName[128];
	bool m_Visible;
};

struct CMapLayerEx
{
	CMapLayerEx(bool Visible, char *pName) : m_Visible(Visible)
		{m_ButtonID = -1; str_copy(m_aName, pName, sizeof(m_aName));}
	int m_ButtonID;
	char m_aName[128];
	bool m_Visible;
};

class CLayers
{
	array<CMapGroupEx *> m_pMapGroups;
	array<CMapLayerEx *> m_pMapLayers;
	int m_GroupsNum;
	int m_GroupsStart;
	int m_LayersNum;
	int m_LayersStart;
	CMapItemGroup *m_pGameGroup;
	CMapItemLayerTilemap *m_pGameLayer;
	class IMap *m_pMap;

public:
	CLayers();
	void Init(class IKernel *pKernel);
	int NumGroups() const { return m_GroupsNum; };
	class IMap *Map() const { return m_pMap; };
	CMapItemGroup *GameGroup() const { return m_pGameGroup; };
	CMapItemLayerTilemap *GameLayer() const { return m_pGameLayer; };
	CMapItemGroup *GetGroup(int Index);
	CMapItemLayer *GetLayer(int Index);

	CMapGroupEx *GetGroupEx(int Index);
	CMapLayerEx *GetLayerEx(int Index);
};

#endif
