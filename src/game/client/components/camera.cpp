/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <base/math.h>
#include <game/collision.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>

#include "camera.h"
#include "controls.h"

CCamera::CCamera()
{
	m_CamType = CAMTYPE_UNDEFINED;
	m_Zoom = 1.0f;
}

void CCamera::OnRender()
{
	//vec2 center;
	if(Client()->State() != IClient::STATE_OFFLINE)
		m_Zoom = 1.0f;

	// update camera center
	if(m_pClient->m_Snap.m_SpecInfo.m_Active && !m_pClient->m_Snap.m_SpecInfo.m_UsePosition)
	{
		if(m_CamType != CAMTYPE_SPEC)
		{
			m_pClient->m_pControls->m_MousePos = m_PrevCenter;
			m_pClient->m_pControls->ClampMousePos();
			m_CamType = CAMTYPE_SPEC;
		}
		m_Center = m_pClient->m_pControls->m_MousePos;
	}
	else if (Client()->State() == IClient::STATE_OFFLINE)
	{
		// sway
		float d = distance(m_Center, m_WantedPos);
		float r = 10.0f;
		float s = 7.5f;

		// movement
		int64 Now = time_get();
		int acc = 1000;
		float t = acc / 0.5f;
		float a = 0;

		if (m_Center != m_WantedPos && !m_Sway)
		{
			a = 1.0f - ((float)clamp(acc - (int)((Now - m_LastMove) / (time_freq() / t)), 0, acc) / acc);
			m_Center = m_PrevPos - (m_PrevPos - m_WantedPos) * a;
			m_Zoom = m_PrevZoom - (m_PrevZoom - m_WantedZoom) * a;
		}
		else
			m_Sway = true;

		if (m_Sway)
		{
			t = acc / s;
			a = 1.0f - ((float)clamp(acc - (int)((Now - m_LastSway) / (time_freq() / t)), 0, acc) / acc);
			m_Center = m_WantedPos + vec2(cosf(2*pi * a), sinf(2*pi * a))*r;
		}

		if (m_LastSway + time_freq() * s < Now)
			m_LastSway = Now;
	}
	else
	{
		if(m_CamType != CAMTYPE_PLAYER)
		{
			m_pClient->m_pControls->ClampMousePos();
			m_CamType = CAMTYPE_PLAYER;
		}

		vec2 CameraOffset(0, 0);

		float l = length(m_pClient->m_pControls->m_MousePos);
		if(l > 0.0001f) // make sure that this isn't 0
		{
			float DeadZone = g_Config.m_ClMouseDeadzone;
			float FollowFactor = g_Config.m_ClMouseFollowfactor/100.0f;
			float OffsetAmount = max(l-DeadZone, 0.0f) * FollowFactor;

			CameraOffset = normalize(m_pClient->m_pControls->m_MousePos)*OffsetAmount;
		}

		if(m_pClient->m_Snap.m_SpecInfo.m_Active)
			m_Center = m_pClient->m_Snap.m_SpecInfo.m_Position + CameraOffset;
		else
			m_Center = m_pClient->m_LocalCharacterPos + CameraOffset;
	}

	m_PrevCenter = m_Center;
}

void CCamera::Move(vec2 Pos, float Zoom, bool Smooth)
{
	static bool First = true;
	Pos *= 32.0f;
	if (First || !Smooth)
	{
		m_WantedPos = Pos;
		m_Zoom = Zoom;
		First = false;
		return;
	}

	if (m_WantedPos != Pos)
	{
		m_PrevPos = m_Center;
		m_WantedPos = Pos;
		m_PrevZoom = m_Zoom;
		m_WantedZoom = Zoom;
		m_LastMove = time_get();
		m_Sway = false;
	}
}