/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_SCOREBOARD_H
#define GAME_CLIENT_COMPONENTS_SCOREBOARD_H
#include <game/client/component.h>

class CScoreboard : public CComponent
{
	void RenderGoals(float x, float y, float w);
	void RenderSpectators(float x, float y, float w);
	void RenderSpectators64(float x, float y, float w);
	void RenderScoreboard(float x, float y, float w, float h, int Team, const char *pTitle, int Corners);
	void RenderScoreboard64(float x, float y, float w, float h, int Team, const char *pTitle, int Corners, int Max, int Start, int *Last, bool First);
	void RenderRecordingNotification(float x);

	static void ConKeyScoreboard(IConsole::IResult *pResult, void *pUserData);

	const char *GetClanName(int Team);

	bool m_Active;
	int m_NumPlayers;
	int m_NumSpectators;

public:
	CScoreboard();
	virtual void OnReset();
	virtual void OnConsoleInit();
	virtual void OnRender();
	virtual void OnRelease();

	bool Active();
};

#endif
