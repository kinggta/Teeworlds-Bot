/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_CAMERA_H
#define GAME_CLIENT_COMPONENTS_CAMERA_H
#include <base/vmath.h>
#include <game/client/component.h>

class CCamera : public CComponent
{
	enum
	{
		CAMTYPE_UNDEFINED=-1,
		CAMTYPE_SPEC,
		CAMTYPE_PLAYER,
		CAMTYPE_MENU,
	};

	vec2 m_WantedPos;
	vec2 m_PrevPos;
	int64 m_LastMove;
	int64 m_LastSway;
	float m_WantedZoom;
	float m_PrevZoom;
	int m_CamType;
	bool m_Sway;

public:
	vec2 m_Center;
	vec2 m_PrevCenter;
	
	float m_Zoom;

	CCamera();
	virtual void OnRender();

	void Move(vec2 Pos, float Zoom, bool Smooth=true);
};

#endif
