/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>

#include <engine/shared/config.h>

#include <game/collision.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>
#include <game/client/components/chat.h>
#include <game/client/components/menus.h>
#include <game/client/components/scoreboard.h>

#include "controls.h"

CControls::CControls()
{
	mem_zero(&m_LastData, sizeof(m_LastData));
}

void CControls::OnReset()
{
	m_LastData.m_Direction = 0;
	m_LastData.m_Hook = 0;
	// simulate releasing the fire button
	if((m_LastData.m_Fire&1) != 0)
		m_LastData.m_Fire++;
	m_LastData.m_Fire &= INPUT_STATE_MASK;
	m_LastData.m_Jump = 0;
	m_InputData = m_LastData;

	m_InputDirectionLeft = 0;
	m_InputDirectionRight = 0;

	m_InputDirectionSlowLeft = 0;
	m_InputDirectionSlowRight = 0;
}

void CControls::OnRelease()
{
	OnReset();
}

void CControls::OnPlayerDeath()
{
	m_LastData.m_WantedWeapon = m_InputData.m_WantedWeapon = 0;
}

static void ConKeyInputState(IConsole::IResult *pResult, void *pUserData)
{
	((int *)pUserData)[0] = pResult->GetInteger(0);
}

static void ConKeyInputCounter(IConsole::IResult *pResult, void *pUserData)
{
	int *v = (int *)pUserData;
	if(((*v)&1) != pResult->GetInteger(0))
		(*v)++;
	*v &= INPUT_STATE_MASK;
}

struct CInputSet
{
	CControls *m_pControls;
	int *m_pVariable;
	int m_Value;
};

static void ConKeyInputSet(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	if(pResult->GetInteger(0))
		*pSet->m_pVariable = pSet->m_Value;
}

static void ConKeyInputNextPrevWeapon(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	ConKeyInputCounter(pResult, pSet->m_pVariable);
	pSet->m_pControls->m_InputData.m_WantedWeapon = 0;
}

void CControls::OnConsoleInit()
{
	// game commands
	Console()->Register("+left", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionLeft, "Move left");
	Console()->Register("+right", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionRight, "Move right");
	Console()->Register("+jump", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Jump, "Jump");
	Console()->Register("+hook", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Hook, "Hook");
	Console()->Register("+fire", "", CFGFLAG_CLIENT, ConKeyInputCounter, &m_InputData.m_Fire, "Fire");
	Console()->Register("+aim", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputAimbot, "Aimbot");
	Console()->Register("+slowright", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionSlowRight, "Move slow right");
	Console()->Register("+slowleft", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionSlowLeft, "Move left right");

	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 1}; Console()->Register("+weapon1", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to hammer"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 2}; Console()->Register("+weapon2", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to gun"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 3}; Console()->Register("+weapon3", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to shotgun"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 4}; Console()->Register("+weapon4", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to grenade"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 5}; Console()->Register("+weapon5", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to rifle"); }

	{ static CInputSet s_Set = {this, &m_InputData.m_NextWeapon, 0}; Console()->Register("+nextweapon", "", CFGFLAG_CLIENT, ConKeyInputNextPrevWeapon, (void *)&s_Set, "Switch to next weapon"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_PrevWeapon, 0}; Console()->Register("+prevweapon", "", CFGFLAG_CLIENT, ConKeyInputNextPrevWeapon, (void *)&s_Set, "Switch to previous weapon"); }
}

void CControls::OnMessage(int Msg, void *pRawMsg)
{
	if(Msg == NETMSGTYPE_SV_WEAPONPICKUP)
	{
		CNetMsg_Sv_WeaponPickup *pMsg = (CNetMsg_Sv_WeaponPickup *)pRawMsg;
		if(g_Config.m_ClAutoswitchWeapons)
			m_InputData.m_WantedWeapon = pMsg->m_Weapon+1;
	}
}

bool CControls::Spinbot()
{
	if(!g_Config.m_XSpinbot)
		return false;
	return false;
}

bool CControls::Aimbot()
{
	if(!g_Config.m_XAimbot || (g_Config.m_XAimbotHotkey && !m_InputAimbot))
		return false;
	
	int numPlayers = 0;
	int LocalID = m_pClient->m_Snap.m_LocalClientID;
	int LocalPing = m_pClient->m_Snap.m_paPlayerInfos[LocalID]->m_Latency;
	int LocalTeam = m_pClient->m_Snap.m_paPlayerInfos[LocalID]->m_Team;
	CNetObj_CharacterCore LocalChar;
	m_pClient->m_aClients[LocalID].m_Predicted.Write(&LocalChar);
	vec2 LocalVel = vec2(LocalChar.m_VelX / 256.0f, LocalChar.m_VelY / 256.0f) * (LocalPing*(50 / 1000));
	vec2 LocalPos = vec2(LocalChar.m_X, LocalChar.m_Y) + LocalVel;
	vec2 TargetPos = vec2(0, 0);
	CArgs_Aimbot Args;

	for(int i = 0, j = 0; i < MAX_CLIENTS; i++)
	{
		vec2 EnemyPos;
		vec2 EnemyVel;
		CGameClient::CSnapState::CCharacterInfo EnemyChar;
		const void *pInfo = NULL;
		int EnemyTeam = 0;
		int Friendstate = 0;

		if(!m_pClient->m_Snap.m_aCharacters[i].m_Active || i == LocalID)
			continue;

		EnemyChar = m_pClient->m_Snap.m_aCharacters[i];
		pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_PLAYERINFO, i);
		if(i == LocalID)
		{
			LocalTeam = ((const CNetObj_PlayerInfo *)pInfo)->m_Team;
			continue;
		}
		EnemyVel = mix(vec2(EnemyChar.m_Prev.m_VelX, EnemyChar.m_Prev.m_VelY), vec2(EnemyChar.m_Cur.m_VelX, EnemyChar.m_Cur.m_VelY), Client()->IntraGameTick()) * (LocalPing*(50 / 1000) * 2) ;
		EnemyPos = mix(vec2(EnemyChar.m_Prev.m_X, EnemyChar.m_Prev.m_Y), vec2(EnemyChar.m_Cur.m_X, EnemyChar.m_Cur.m_Y), Client()->IntraGameTick()) + EnemyVel;
		EnemyTeam = ((const CNetObj_PlayerInfo *)pInfo)->m_Team;
		Friendstate = m_pClient->Friends()->IsFriend(m_pClient->m_aClients[i].m_aName, m_pClient->m_aClients[i].m_aClan, true);

		if(g_Config.m_XAimbotWallcheck && Collision()->IntersectLine(LocalPos, EnemyPos, 0, 0)) // Wallcheck
			continue;
		if(g_Config.m_XAimbotRange <= distance(EnemyPos, LocalPos)) // Aimrange
			continue;
		if(g_Config.m_XAimbotTeamcheck && LocalTeam == EnemyTeam && (m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS))) // Teamcheck
			continue;
		if(g_Config.m_XAimbotEnemycheck && Friendstate != IFriends::CONTACT_ENEMY) // Aim on enemy only
			continue;
		if(g_Config.m_XAimbotFriendcheck && Friendstate == IFriends::CONTACT_FRIEND) // Do not aim on friends
			continue;

		dbg_msg(0, m_pClient->m_aClients[i].m_aSkinName);
		if(g_Config.m_XAimbotFNG && EnemyChar.m_Cur.m_Weapon%NUM_WEAPONS == WEAPON_NINJA)
			continue;

		Args.m_aEnemyPos[j++] = EnemyPos;
		numPlayers++;
	}

	if(numPlayers)
	{
		Args.m_LocalPos = LocalPos;
		Args.m_MousePos = m_MousePos;
		Args.m_pOut = &TargetPos;
		Args.m_numPlayers = numPlayers;
		Args.m_Bypass = g_Config.m_XAimbotBypass;
		Client()->m_LibHandler.CallFunction[CLibHandler::LIB_USER](CLibHandler::CALL_AIMBOT, &Args);

		m_InputData.m_TargetX = (int)Args.m_pOut->x;
		m_InputData.m_TargetY = (int)Args.m_pOut->y;
		return true;
	}
	return false;
}

bool CControls::Triggerbot()
{
	if(!g_Config.m_XTriggerbot || (g_Config.m_XAimbotHotkey && !m_InputAimbot))
		return false;

	int LocalID = m_pClient->m_Snap.m_LocalClientID;
	int LocalPing = m_pClient->m_Snap.m_paPlayerInfos[LocalID]->m_Latency;
	int LocalTeam = m_pClient->m_Snap.m_paPlayerInfos[LocalID]->m_Team;
	CGameClient::CSnapState::CCharacterInfo LocalChar = m_pClient->m_Snap.m_aCharacters[LocalID];
	vec2 LocalVel = vec2(LocalChar.m_Cur.m_VelX, LocalChar.m_Cur.m_VelY) * (LocalPing*(50 / 1000));
	vec2 LocalPos = vec2(LocalChar.m_Cur.m_X, LocalChar.m_Cur.m_Y) + LocalVel;

	for(int i = 0, j = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_pClient->m_Snap.m_aCharacters[i].m_Active || i == LocalID)
			continue;
	
		const void *pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_PLAYERINFO, i);;
		CGameClient::CSnapState::CCharacterInfo EnemyChar = m_pClient->m_Snap.m_aCharacters[i];
		vec2 EnemyVel = vec2(EnemyChar.m_Cur.m_VelX, EnemyChar.m_Cur.m_VelY) * (LocalPing*(50 / 1000));
		vec2 EnemyPos = vec2(EnemyChar.m_Cur.m_X, EnemyChar.m_Cur.m_Y) + EnemyVel;
		int EnemyTeam = ((const CNetObj_PlayerInfo *)pInfo)->m_Team;

		vec2 To = LocalPos + normalize(vec2(m_InputData.m_TargetX, m_InputData.m_TargetY)) * g_Config.m_XAimbotRange;
		vec2 IntersectPos = closest_point_on_line(LocalPos, To, EnemyPos);

		int Hit = Collision()->IntersectLine(LocalPos, EnemyPos, 0, 0);

		if(g_Config.m_XAimbotWallcheck && Hit) // Wallcheck
			continue;
		if(g_Config.m_XAimbotRange <= distance(EnemyPos, LocalPos)) // Aimrange
			continue;
		if(g_Config.m_XAimbotTeamcheck && LocalTeam == EnemyTeam && (m_pClient->m_Snap.m_pGameInfoObj && (m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)))
			continue;
	
		
		if(distance(EnemyPos, IntersectPos) < 28)
		{
			m_InputData.m_Fire += 2;
			return true;
		}
	}
	return false;
}

bool CControls::Bot()
{
	if(Aimbot())
		return true;
	if(Spinbot())
		return true;
	return false;
}
int CControls::SnapInput(int *pData)
{
	static int64 LastSendTime = 0;
	static int64 LastPingTime = 0;
	bool Send = false;

	// update player state
	if(m_pClient->m_pChat->IsActive())
		m_InputData.m_PlayerFlags = PLAYERFLAG_CHATTING;
	else if(m_pClient->m_pMenus->IsActive())
		m_InputData.m_PlayerFlags = PLAYERFLAG_IN_MENU;
	else
		m_InputData.m_PlayerFlags = PLAYERFLAG_PLAYING;

	if(m_pClient->m_pScoreboard->Active() || time_get() > LastPingTime + time_freq())
	{
		m_InputData.m_PlayerFlags |= PLAYERFLAG_SCOREBOARD;
		LastPingTime = time_get();
	}

	if(m_LastData.m_PlayerFlags != m_InputData.m_PlayerFlags)
		Send = true;

	m_LastData.m_PlayerFlags = m_InputData.m_PlayerFlags;

	// we freeze the input if chat or menu is activated
	if(!(m_InputData.m_PlayerFlags&PLAYERFLAG_PLAYING))
	{
		if(m_InputData.m_PlayerFlags&PLAYERFLAG_CHATTING)
			OnReset();

		mem_copy(pData, &m_InputData, sizeof(m_InputData));

		// send once a second just to be sure
		if(time_get() > LastSendTime + time_freq())
			Send = true;
	}
	else
	{
		if (!Bot())
		{
			m_InputData.m_TargetX = (int)m_MousePos.x;
			m_InputData.m_TargetY = (int)m_MousePos.y;
		}
		Triggerbot();

		if(!m_InputData.m_TargetX && !m_InputData.m_TargetY)
		{
			m_InputData.m_TargetX = 1;
			m_MousePos.x = 1;
		}

		// set direction
		m_InputData.m_Direction = 0;
		if(m_InputDirectionLeft && !m_InputDirectionRight)
			m_InputData.m_Direction = -1;
		if(!m_InputDirectionLeft && m_InputDirectionRight)
			m_InputData.m_Direction = 1;


		// dont question this code.. it works
		static int FirstMove = 0;
		if(m_InputDirectionSlowLeft)// slow left
		{
			FirstMove++;
			if(FirstMove == 1)
				m_InputData.m_Direction = -1;
		}
		else if(m_InputDirectionSlowRight) // slow right
		{
			FirstMove++;
			if(FirstMove == 1)
				m_InputData.m_Direction = 1;
		}
		m_InputDirectionSlowLeft = 0;
		m_InputDirectionSlowRight = 0;
		FirstMove = 0;


		// stress testing
		if(g_Config.m_DbgStress)
		{
			float t = Client()->LocalTime();
			mem_zero(&m_InputData, sizeof(m_InputData));

			m_InputData.m_Direction = ((int)t/2)&1;
			m_InputData.m_Jump = ((int)t);
			m_InputData.m_Fire = ((int)(t*10));
			m_InputData.m_Hook = ((int)(t*2))&1;
			m_InputData.m_WantedWeapon = ((int)t)%NUM_WEAPONS;
			m_InputData.m_TargetX = (int)(sinf(t*3)*100.0f);
			m_InputData.m_TargetY = (int)(cosf(t*3)*100.0f);
		}

		// check if we need to send input
		if(m_InputData.m_Direction != m_LastData.m_Direction) Send = true;
		else if(m_InputData.m_Jump != m_LastData.m_Jump) Send = true;
		else if(m_InputData.m_Fire != m_LastData.m_Fire) Send = true;
		else if(m_InputData.m_Hook != m_LastData.m_Hook) Send = true;
		else if(m_InputData.m_WantedWeapon != m_LastData.m_WantedWeapon) Send = true;
		else if(m_InputData.m_NextWeapon != m_LastData.m_NextWeapon) Send = true;
		else if(m_InputData.m_PrevWeapon != m_LastData.m_PrevWeapon) Send = true;

		// send at at least 10hz
		if(time_get() > LastSendTime + time_freq()/25)
			Send = true;
	}

	// copy and return size
	m_LastData = m_InputData;

	if(!Send)
		return 0;

	LastSendTime = time_get();
	mem_copy(pData, &m_InputData, sizeof(m_InputData));
	return sizeof(m_InputData);
}

void CControls::OnRender()
{
	// update target pos
	if(m_pClient->m_Snap.m_pGameInfoObj && !m_pClient->m_Snap.m_SpecInfo.m_Active)
		m_TargetPos = m_pClient->m_LocalCharacterPos + m_MousePos;
	else if(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_Snap.m_SpecInfo.m_UsePosition)
		m_TargetPos = m_pClient->m_Snap.m_SpecInfo.m_Position + m_MousePos;
	else
		m_TargetPos = m_MousePos;
}

bool CControls::OnMouseMove(float x, float y)
{
	if((m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED) ||
		(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_pChat->IsActive()))
		return false;

	m_MousePos += vec2(x, y); // TODO: ugly
	ClampMousePos();

	return true;
}

void CControls::ClampMousePos()
{
	if(m_pClient->m_Snap.m_SpecInfo.m_Active && !m_pClient->m_Snap.m_SpecInfo.m_UsePosition)
	{
		m_MousePos.x = clamp(m_MousePos.x, 200.0f, Collision()->GetWidth()*32-200.0f);
		m_MousePos.y = clamp(m_MousePos.y, 200.0f, Collision()->GetHeight()*32-200.0f);

	}
	else
	{
		float CameraMaxDistance = 200.0f;
		float FollowFactor = g_Config.m_ClMouseFollowfactor/100.0f;
		float MouseMax = min(CameraMaxDistance/FollowFactor + g_Config.m_ClMouseDeadzone, (float)g_Config.m_ClMouseMaxDistance);

		if(length(m_MousePos) > MouseMax)
			m_MousePos = normalize(m_MousePos)*MouseMax;
	}
}
