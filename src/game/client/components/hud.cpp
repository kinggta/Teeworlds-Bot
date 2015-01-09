/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>
#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <game/client/render.h>

#include <game/client/components/skins.h> //

#include "controls.h"
#include "camera.h"
#include "hud.h"
#include "voting.h"
#include "binds.h"

CHud::CHud()
{
	// won't work if zero
	m_AverageFPS = 1.0f;
}

void CHud::OnReset()
{
}

void CHud::RenderGameTimer()
{
	float Half = 300.0f*Graphics()->ScreenAspect()/2.0f;

	if(!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH))
	{
		char Buf[32];
		int Time = 0;
		if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit && !m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
		{
			Time = m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit*60 - ((Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed());

			if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
				Time = 0;
		}
		else
			Time = (Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed();

		str_format(Buf, sizeof(Buf), "%d:%02d", Time/60, Time%60);
		float FontSize = 10.0f;
		float w = TextRender()->TextWidth(0, FontSize, Buf, -1);
		// last 60 sec red, last 10 sec blink
		if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit && Time <= 60 && !m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
		{
			float Alpha = Time <= 10 && (2*time_get()/time_freq()) % 2 ? 0.5f : 1.0f;
			TextRender()->TextColor(1.0f, 0.25f, 0.25f, Alpha);
		}
		TextRender()->Text(0, Half-w/2, 2, FontSize, Buf, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void CHud::RenderPauseNotification()
{
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED &&
		!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
	{
		const char *pText = Localize("Game paused");
		float FontSize = 20.0f;
		float w = TextRender()->TextWidth(0, FontSize,pText, -1);
		TextRender()->Text(0, 150.0f*Graphics()->ScreenAspect()+-w/2.0f, 50.0f, FontSize, pText, -1);
	}
}

void CHud::RenderSuddenDeath()
{
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH)
	{
		float Half = 300.0f*Graphics()->ScreenAspect()/2.0f;
		const char *pText = Localize("Sudden Death");
		float FontSize = 12.0f;
		float w = TextRender()->TextWidth(0, FontSize, pText, -1);
		TextRender()->Text(0, Half-w/2, 2, FontSize, pText, -1);
	}
}

void CHud::RenderSXRadar()
{
	int Size = 23.0f + TextRender()->TextWidth(0, 14.0f, "100", -1);
	CUIRect r = { m_Width-Size, 225-Size, Size, Size };
	vec2 c = vec2(r.x+r.w*0.5f, r.y+r.h*0.5f);

	//draw background
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0.2f,0,0.5f);
	RenderTools()->DrawRoundRectExt(r.x, r.y, r.w, r.h, 5.0f, CUI::CORNER_L);
	Graphics()->QuadsEnd();

	//draw local character
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f, 1.0f, 0.0f, 1.0f); // green
	IGraphics::CQuadItem QuadItem(c.x, c.y, 2, 2);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

	if(!m_pClient->m_Snap.m_pLocalCharacter || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		// only render active characters
		if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
			continue;
		//skip local character
		if(i == m_pClient->m_Snap.m_LocalClientID)
			continue;
			
		CNetObj_Character pCurChar = m_pClient->m_Snap.m_aCharacters[i].m_Cur;
		
		vec2 d = vec2(m_pClient->m_Snap.m_pLocalCharacter->m_X - pCurChar.m_X, m_pClient->m_Snap.m_pLocalCharacter->m_Y - pCurChar.m_Y);

		if(d.x > 800.0f || d.y > 800.0f)
			continue;
		//if(length(d) > 800.0f)
		//	continue;


		Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1.0f, 0.0f, 0.0f,  1.0f); // red
		if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS && m_pClient->m_aClients[i].m_Team == TEAM_BLUE)
			Graphics()->SetColor(0.0f, 0.0f, 1.0f, 1.0f); // blue
		IGraphics::CQuadItem QuadItem(c.x - d.x/r.w, c.y - d.y/r.h, 2, 2);
		Graphics()->QuadsDrawTL(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}
}

void CHud::RenderSXPing()
{
	char Buf[32];
	int Latency = -1;
	float Whole = 300*Graphics()->ScreenAspect();
	float x = Whole - 2.0f - 6*4;
	float y = m_Height-7.0f;


	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_paPlayerInfos[i];
		if(pInfo && pInfo->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
		{
			if(pInfo->m_Team == TEAM_SPECTATORS)
				return; //no ping for specs
			Latency = pInfo->m_Latency;
			break;
		}
	}

	int n = 0;
	vec3 c = vec3(0.2f, 1.0f, 0.2f);
	if(Latency <= 40)
	{
		n = 5;
		c = vec3(0.2f, 1.0f, 0.2f);
	}
	else if(Latency <= 60)
	{
		n = 4;
		c = vec3(0.4f, 0.8f, 0.2f);
	}
	else if(Latency <= 85)
	{
		n = 3;
		c = vec3(0.6f, 0.6f, 0.2f);
	}
	else if(Latency <= 130)
	{
		n = 2;
		c = vec3(0.8f, 0.4f, 0.2f);
	}
	else if(Latency <= 180)
	{
		n = 1;
		c = vec3(1.0f, 0.2f, 0.2f);
	}

	str_format(Buf, sizeof(Buf), "%d", Latency);
	float tw = TextRender()->TextWidth(0, 8.0f, Buf, -1);

	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);	
	TextRender()->Text(0, x-tw-5.0f, y-6.5f, 8.0f, Buf, -1);


	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	
	Graphics()->SetColor(0, 0, 0, 0.75f);
	for(int i = n; i < 5; i++)
		RenderTools()->DrawRoundRect(x+i*4, y-i*2, 2, 2+i*2, 0.0f);

	Graphics()->SetColor(c.x, c.y, c.z, 1.0f);
	for(int i = 0; i < n; i++)
		RenderTools()->DrawRoundRect(x+i*4, y-i*2, 2, 2+i*2, 0.0f);
		
	Graphics()->QuadsEnd();
}

void CHud::RenderSXScoreHud()
{
	float ScoreLimit = m_pClient->m_Snap.m_pGameInfoObj->m_ScoreLimit;
	float Half = 300.0f*Graphics()->ScreenAspect()/2.0f;
	float FontSize = 8.0f;
	float w = 100.0f, h = 12.0f, x = Half-w/2, y = m_Height-24.0f, hw = w/2;
	float CornerWidth = TextRender()->TextWidth(0, 8.0f, "99999", -1);
	char Buf[32];
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	
	// background
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
	RenderTools()->DrawRoundRect(x, y, w, h, 0.0f); // body
	RenderTools()->DrawRoundRect(x, y+2, w, h-4, 1.0f); // scorebars
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.6f);
	RenderTools()->DrawRoundRectExt(x-10, y+h, w+20, h*0.5f, 2.0f, CUI::CORNER_B); // names
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
	
	// Corners
	RenderTools()->DrawRoundRectExt(x-CornerWidth, y, CornerWidth, h, 4.0f, CUI::CORNER_L);
	RenderTools()->DrawRoundRectExt(x+w, y, CornerWidth, h, 4.0f, CUI::CORNER_R);

	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS && m_pClient->m_Snap.m_pGameDataObj)
	{
		float TeamScore[2] = {m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed, m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue};
		int FlagCarrier[2] = { m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierRed, m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierBlue };	
		
		if(ScoreLimit)
		{
			// red score bar
			float bw = TeamScore[0] / ScoreLimit * hw;
			bw = clamp(bw, 0.0f, hw);
			Graphics()->SetColor(1.0f, 0.2f, 0.2f, 0.75f);
			RenderTools()->DrawRoundRectExt(x, y+2, bw, h-4, bw >= 1.0f ? 1.0f : 0.0f,  CUI::CORNER_L);

			// blue score bar
			bw = TeamScore[1] / ScoreLimit * hw;
			bw = clamp(bw, 0.0f, hw);
			Graphics()->SetColor(0.2f, 0.2f, 1.0f, 0.75f);
			RenderTools()->DrawRoundRectExt(Half+hw-bw, y+2, bw, h-4, bw >= 1.0f ? 1.0f : 0.0f, CUI::CORNER_R);		
		}

		// middle / scorelimit
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderTools()->DrawRoundRect(Half-0.5f, y+2, 1, h-4, 0.0f);
		Graphics()->QuadsEnd();

		// red score
		str_format(Buf, sizeof(Buf), "%d", (int)TeamScore[0]);
		float tw = TextRender()->TextWidth(0, 8.0f, Buf, -1);
		TextRender()->TextColor(1.0f, 0.5f, 0.5f, 1.0f);	
		TextRender()->Text(0, x-tw-1, y+0.5f, 8.0f, Buf, -1);
	
		// blue score
		str_format(Buf, sizeof(Buf), "%d", (int)TeamScore[1]);
		tw = TextRender()->TextWidth(0, 8.0f, Buf, -1);
		TextRender()->TextColor(0.7f, 0.7f, 1.0f, 1.0f);
		TextRender()->Text(0, x+w+1, y+0.5f, 8.0f, Buf, -1);

		// scorelimit
		/*str_format(Buf, sizeof(Buf), "%d", (int)ScoreLimit);
		tw = TextRender()->TextWidth(0, FontSize, Buf, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
		TextRender()->Text(0, Half-tw/2, y+h*1.5f, FontSize, Buf, -1);*/
	}
	else
	{
		int Local = -1;
		int aPos[2] = { 1, 2 };
		const CNetObj_PlayerInfo *apPlayerInfo[2] = { 0, 0 };
		int i = 0;
		for(int t = 0; t < 2 && i < MAX_CLIENTS && m_pClient->m_Snap.m_paInfoByScore[i]; ++i)
		{
			if(m_pClient->m_Snap.m_paInfoByScore[i]->m_Team != TEAM_SPECTATORS)
			{
				apPlayerInfo[t] = m_pClient->m_Snap.m_paInfoByScore[i];
				if(apPlayerInfo[t]->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
					Local = t;
				++t;
			}
		}
		// search local player info if not a spectator, nor within top2 scores
		if(Local == -1 && m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team != TEAM_SPECTATORS)
		{
			for(; i < MAX_CLIENTS && m_pClient->m_Snap.m_paInfoByScore[i]; ++i)
			{
				if(m_pClient->m_Snap.m_paInfoByScore[i]->m_Team != TEAM_SPECTATORS)
					++aPos[1];
				if(m_pClient->m_Snap.m_paInfoByScore[i]->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
				{
					apPlayerInfo[1] = m_pClient->m_Snap.m_paInfoByScore[i];
					Local = 1;
					break;
				}
			}
		}

		// make sure local player is on right side of hud
		if(Local == 1)
		{
			const CNetObj_PlayerInfo *tmpInfo = apPlayerInfo[1];
			apPlayerInfo[1] = apPlayerInfo[0];
			apPlayerInfo[0] = tmpInfo;
		}

		if(ScoreLimit)
		{
			float bw = 0;
			// 1st score bar
			if(apPlayerInfo[0])
			{
				bw = clamp(apPlayerInfo[0]->m_Score / ScoreLimit * hw, 0.0f, hw);
				vec3 c = g_Config.m_PlayerUseCustomColor ? m_pClient->m_pSkins->GetColorV3(g_Config.m_PlayerColorBody) :  vec3(1.0f, 1.0f, 1.0f);
				Graphics()->SetColor(c.r, c.g, c.b, 0.75f);
				RenderTools()->DrawRoundRectExt(x, y+2, bw, h-4, bw >= 1.0f ? 1.0f : 0.0f, CUI::CORNER_L);
			}

			// 2nd score bar
			if(apPlayerInfo[1])
			{
				bw = clamp(apPlayerInfo[1]->m_Score / ScoreLimit * hw, 0.0f, hw);

				Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.75f);
				RenderTools()->DrawRoundRectExt(Half+hw-bw, y+2, bw, h-4, bw >= 1.0f ? 1.0f : 0.0f, CUI::CORNER_R);
			}
		}

		// middle / scorelimit
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderTools()->DrawRoundRect(Half-0.5f, y+2, 1, h-4, 0.0f);
		Graphics()->QuadsEnd();


		// 1st score
		float tw = 0;
		if(apPlayerInfo[0])
		{
			str_format(Buf, sizeof(Buf), "%d", apPlayerInfo[0]->m_Score);
			tw = TextRender()->TextWidth(0, 8.0f, Buf, -1);
			vec3 c = g_Config.m_PlayerUseCustomColor ? m_pClient->m_pSkins->GetColorV3(g_Config.m_PlayerColorBody) :  vec3(1.0f, 1.0f, 1.0f);
			TextRender()->TextColor(c.r, c.g, c.b, 1.0f);	
			TextRender()->Text(0, x-tw-1, y+0.5f, 8.0f, Buf, -1);

			int ID = apPlayerInfo[0]->m_ClientID;
			const char *pName = m_pClient->m_aClients[ID].m_aName;
			tw = TextRender()->TextWidth(0, 5.0f, pName, -1);
			TextRender()->TextColor(0.75f, 0.75f, 0.75f, 1.0f);	
			TextRender()->Text(0, x-10+2, y+h-0.5f, 5.0f, pName, -1);

		
		}

		// 2nd score
		if(apPlayerInfo[1])
		{
			str_format(Buf, sizeof(Buf), "%d", apPlayerInfo[1]->m_Score);
			tw = TextRender()->TextWidth(0, 8.0f, Buf, -1);
			TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
			TextRender()->Text(0, x+w+1, y+0.5f, 8.0f, Buf, -1);

			int ID = apPlayerInfo[1]->m_ClientID;
			const char *pName = m_pClient->m_aClients[ID].m_aName;
			tw = TextRender()->TextWidth(0, 5.0f, pName, -1);
			TextRender()->TextColor(0.75f, 0.75f, 0.75f, 1.0f);	
			TextRender()->Text(0, x+w+10-2-tw, y+h-0.5f, 5.0f, pName, -1);
		}

	}
	// scorelimit
		
	str_format(Buf, sizeof(Buf), "%d", (int)ScoreLimit);
	float tw = TextRender()->TextWidth(0, FontSize, Buf, -1);
	float tmp = tw + 8 + 6; // textwidth + flagsize + spaces

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.5f);
	RenderTools()->DrawRoundRectExt(Half-tmp*0.5f, y-FontSize-2.0f, tmp, FontSize+2.0f, 2.0f, CUI::CORNER_T);
	

	tmp = Half-tmp*0.5f+2.0f;
	int col = 1;

	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 3; j++)
		{
			Graphics()->SetColor(col,col,col,col+0.5f);
			RenderTools()->DrawRoundRect(tmp+i*2.0f, y-FontSize+j*2.0f, 2, 2, 0.0f);
			col ^= 1;
		}
	Graphics()->QuadsEnd();


	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	TextRender()->Text(0, tmp+10, y-FontSize-2.0f, FontSize, Buf, -1);

}

void CHud::RenderScoreHud()
{
	// render small score hud
	if(!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
	{
		int GameFlags = m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags;
		float Whole = 300*Graphics()->ScreenAspect();
		float StartY = 229.0f;

		if(GameFlags&GAMEFLAG_TEAMS && m_pClient->m_Snap.m_pGameDataObj)
		{
			char aScoreTeam[2][32];
			str_format(aScoreTeam[TEAM_RED], sizeof(aScoreTeam)/2, "%d", m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed);
			str_format(aScoreTeam[TEAM_BLUE], sizeof(aScoreTeam)/2, "%d", m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue);
			float aScoreTeamWidth[2] = { TextRender()->TextWidth(0, 14.0f, aScoreTeam[TEAM_RED], -1), TextRender()->TextWidth(0, 14.0f, aScoreTeam[TEAM_BLUE], -1) };
			int FlagCarrier[2] = { m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierRed, m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierBlue };
			float ScoreWidthMax = max(max(aScoreTeamWidth[TEAM_RED], aScoreTeamWidth[TEAM_BLUE]), TextRender()->TextWidth(0, 14.0f, "100", -1));
			float Split = 3.0f;
			float ImageSize = GameFlags&GAMEFLAG_FLAGS ? 16.0f : Split;

			for(int t = 0; t < 2; t++)
			{
				// draw box
				Graphics()->BlendNormal();
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				if(t == 0)
					Graphics()->SetColor(1.0f, 0.0f, 0.0f, 0.25f);
				else
					Graphics()->SetColor(0.0f, 0.0f, 1.0f, 0.25f);
				RenderTools()->DrawRoundRectExt(Whole-ScoreWidthMax-ImageSize-2*Split, StartY+t*20, ScoreWidthMax+ImageSize+2*Split, 18.0f, 5.0f, CUI::CORNER_L);
				Graphics()->QuadsEnd();

				// draw score
				TextRender()->Text(0, Whole-ScoreWidthMax+(ScoreWidthMax-aScoreTeamWidth[t])/2-Split, StartY+t*20, 14.0f, aScoreTeam[t], -1);

				if(GameFlags&GAMEFLAG_FLAGS)
				{
					int BlinkTimer = (m_pClient->m_FlagDropTick[t] != 0 &&
										(Client()->GameTick()-m_pClient->m_FlagDropTick[t])/Client()->GameTickSpeed() >= 25) ? 10 : 20;
					if(FlagCarrier[t] == FLAG_ATSTAND || (FlagCarrier[t] == FLAG_TAKEN && ((Client()->GameTick()/BlinkTimer)&1)))
					{
						// draw flag
						Graphics()->BlendNormal();
						Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
						Graphics()->QuadsBegin();
						RenderTools()->SelectSprite(t==0?SPRITE_FLAG_RED:SPRITE_FLAG_BLUE);
						IGraphics::CQuadItem QuadItem(Whole-ScoreWidthMax-ImageSize, StartY+1.0f+t*20, ImageSize/2, ImageSize);
						Graphics()->QuadsDrawTL(&QuadItem, 1);
						Graphics()->QuadsEnd();
					}
					else if(FlagCarrier[t] >= 0)
					{
						// draw name of the flag holder
						int ID = FlagCarrier[t]%MAX_CLIENTS;
						const char *pName = m_pClient->m_aClients[ID].m_aName;
						float w = TextRender()->TextWidth(0, 8.0f, pName, -1);
						TextRender()->Text(0, min(Whole-w-1.0f, Whole-ScoreWidthMax-ImageSize-2*Split), StartY+(t+1)*20.0f-3.0f, 8.0f, pName, -1);

						// draw tee of the flag holder
						CTeeRenderInfo Info = m_pClient->m_aClients[ID].m_RenderInfo;
						Info.m_Size = 18.0f;
						RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1,0),
							vec2(Whole-ScoreWidthMax-Info.m_Size/2-Split, StartY+1.0f+Info.m_Size/2+t*20));
					}
				}
				StartY += 8.0f;
			}
		}
		else
		{
			int Local = -1;
			int aPos[2] = { 1, 2 };
			const CNetObj_PlayerInfo *apPlayerInfo[2] = { 0, 0 };
			int i = 0;
			for(int t = 0; t < 2 && i < MAX_CLIENTS && m_pClient->m_Snap.m_paInfoByScore[i]; ++i)
			{
				if(m_pClient->m_Snap.m_paInfoByScore[i]->m_Team != TEAM_SPECTATORS)
				{
					apPlayerInfo[t] = m_pClient->m_Snap.m_paInfoByScore[i];
					if(apPlayerInfo[t]->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
						Local = t;
					++t;
				}
			}
			// search local player info if not a spectator, nor within top2 scores
			if(Local == -1 && m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team != TEAM_SPECTATORS)
			{
				for(; i < MAX_CLIENTS && m_pClient->m_Snap.m_paInfoByScore[i]; ++i)
				{
					if(m_pClient->m_Snap.m_paInfoByScore[i]->m_Team != TEAM_SPECTATORS)
						++aPos[1];
					if(m_pClient->m_Snap.m_paInfoByScore[i]->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
					{
						apPlayerInfo[1] = m_pClient->m_Snap.m_paInfoByScore[i];
						Local = 1;
						break;
					}
				}
			}
			char aScore[2][32];
			for(int t = 0; t < 2; ++t)
			{
				if(apPlayerInfo[t])
					str_format(aScore[t], sizeof(aScore)/2, "%d", apPlayerInfo[t]->m_Score);
				else
					aScore[t][0] = 0;
			}
			float aScoreWidth[2] = {TextRender()->TextWidth(0, 14.0f, aScore[0], -1), TextRender()->TextWidth(0, 14.0f, aScore[1], -1)};
			float ScoreWidthMax = max(max(aScoreWidth[0], aScoreWidth[1]), TextRender()->TextWidth(0, 14.0f, "10", -1));
			float Split = 3.0f, ImageSize = 16.0f, PosSize = 16.0f;

			for(int t = 0; t < 2; t++)
			{
				// draw box
				Graphics()->BlendNormal();
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				if(t == Local)
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.25f);
				else
					Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.25f);
				RenderTools()->DrawRoundRectExt(Whole-ScoreWidthMax-ImageSize-2*Split-PosSize, StartY+t*20, ScoreWidthMax+ImageSize+2*Split+PosSize, 18.0f, 5.0f, CUI::CORNER_L);
				Graphics()->QuadsEnd();

				// draw score
				TextRender()->Text(0, Whole-ScoreWidthMax+(ScoreWidthMax-aScoreWidth[t])/2-Split, StartY+t*20, 14.0f, aScore[t], -1);

				if(apPlayerInfo[t])
 				{
					// draw name
					int ID = apPlayerInfo[t]->m_ClientID;
					const char *pName = m_pClient->m_aClients[ID].m_aName;
					float w = TextRender()->TextWidth(0, 8.0f, pName, -1);
					TextRender()->Text(0, min(Whole-w-1.0f, Whole-ScoreWidthMax-ImageSize-2*Split-PosSize), StartY+(t+1)*20.0f-3.0f, 8.0f, pName, -1);

					// draw tee
					CTeeRenderInfo Info = m_pClient->m_aClients[ID].m_RenderInfo;
 					Info.m_Size = 18.0f;
 					RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1,0),
 						vec2(Whole-ScoreWidthMax-Info.m_Size/2-Split, StartY+1.0f+Info.m_Size/2+t*20));
				}

				// draw position
				char aBuf[32];
				str_format(aBuf, sizeof(aBuf), "%d.", aPos[t]);
				TextRender()->Text(0, Whole-ScoreWidthMax-ImageSize-Split-PosSize, StartY+2.0f+t*20, 10.0f, aBuf, -1);

				StartY += 8.0f;
			}
		}
	}
}

void CHud::RenderWarmupTimer()
{
	// render warmup timer
	if(m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
	{
		char Buf[256];
		float FontSize = 20.0f;
		float w = TextRender()->TextWidth(0, FontSize, Localize("Warmup"), -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()+-w/2, 50, FontSize, Localize("Warmup"), -1);

		int Seconds = m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer/SERVER_TICK_SPEED;
		if(Seconds < 5)
			str_format(Buf, sizeof(Buf), "%d.%d", Seconds, (m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer*10/SERVER_TICK_SPEED)%10);
		else
			str_format(Buf, sizeof(Buf), "%d", Seconds);
		w = TextRender()->TextWidth(0, FontSize, Buf, -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()+-w/2, 75, FontSize, Buf, -1);
	}
}

void CHud::MapscreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup)
{
	float Points[4];
	RenderTools()->MapscreenToWorld(CenterX, CenterY, pGroup->m_ParallaxX/100.0f, pGroup->m_ParallaxY/100.0f,
		pGroup->m_OffsetX, pGroup->m_OffsetY, Graphics()->ScreenAspect(), 1.0f, Points);
	Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
}

void CHud::RenderFps()
{
	if(g_Config.m_ClShowfps)
	{
		// calculate avg. fps
		float FPS = 1.0f / Client()->RenderFrameTime();
		m_AverageFPS = (m_AverageFPS*(1.0f-(1.0f/m_AverageFPS))) + (FPS*(1.0f/m_AverageFPS));
		char Buf[512];
		str_format(Buf, sizeof(Buf), "%d", (int)m_AverageFPS);
		TextRender()->Text(0, m_Width-10-TextRender()->TextWidth(0,12,Buf,-1), 5, 12, Buf, -1);
	}
}

void CHud::RenderConnectionWarning()
{
	if(Client()->ConnectionProblems())
	{
		const char *pText = Localize("Connection Problems...");
		float w = TextRender()->TextWidth(0, 24, pText, -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()-w/2, 50, 24, pText, -1);
	}
}

void CHud::RenderTeambalanceWarning()
{
	// render prompt about team-balance
	bool Flash = time_get()/(time_freq()/2)%2 == 0;
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
	{
		int TeamDiff = m_pClient->m_Snap.m_aTeamSize[TEAM_RED]-m_pClient->m_Snap.m_aTeamSize[TEAM_BLUE];
		if (g_Config.m_ClWarningTeambalance && (TeamDiff >= 2 || TeamDiff <= -2))
		{
			const char *pText = Localize("Please balance teams!");
			if(Flash)
				TextRender()->TextColor(1,1,0.5f,1);
			else
				TextRender()->TextColor(0.7f,0.7f,0.2f,1.0f);
			TextRender()->Text(0x0, 5, 50, 6, pText, -1);
			TextRender()->TextColor(1,1,1,1);
		}
	}
}


void CHud::RenderVoting()
{
	if(!m_pClient->m_pVoting->IsVoting() || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.40f);
	RenderTools()->DrawRoundRect(-10, 60-2, 100+10+4+5, 46, 5.0f);
	Graphics()->QuadsEnd();

	TextRender()->TextColor(1,1,1,1);

	CTextCursor Cursor;
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), Localize("%ds left"), m_pClient->m_pVoting->SecondsLeft());
	float tw = TextRender()->TextWidth(0x0, 6, aBuf, -1);
	TextRender()->SetCursor(&Cursor, 5.0f+100.0f-tw, 60.0f, 6.0f, TEXTFLAG_RENDER);
	TextRender()->TextEx(&Cursor, aBuf, -1);

	TextRender()->SetCursor(&Cursor, 5.0f, 60.0f, 6.0f, TEXTFLAG_RENDER);
	Cursor.m_LineWidth = 100.0f-tw;
	Cursor.m_MaxLines = 3;
	TextRender()->TextEx(&Cursor, m_pClient->m_pVoting->VoteDescription(), -1);

	// reason
	str_format(aBuf, sizeof(aBuf), "%s %s", Localize("Reason:"), m_pClient->m_pVoting->VoteReason());
	TextRender()->SetCursor(&Cursor, 5.0f, 79.0f, 6.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
	Cursor.m_LineWidth = 100.0f;
	TextRender()->TextEx(&Cursor, aBuf, -1);

	CUIRect Base = {5, 88, 100, 4};
	m_pClient->m_pVoting->RenderBars(Base, true);

	const char *pYesKey = m_pClient->m_pBinds->GetKey("vote yes");
	const char *pNoKey = m_pClient->m_pBinds->GetKey("vote no");
	str_format(aBuf, sizeof(aBuf), "%s - %s", pYesKey, Localize("Vote yes"));
	Base.y += Base.h+1;
	UI()->DoLabel(&Base, aBuf, 6.0f, -1);

	str_format(aBuf, sizeof(aBuf), "%s - %s", Localize("Vote no"), pNoKey);
	UI()->DoLabel(&Base, aBuf, 6.0f, 1);
}

void CHud::OnHit()
{
	m_LastHit = time_get();
}

void CHud::RenderCursor()
{
	if(!m_pClient->m_Snap.m_pLocalCharacter || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	MapscreenToGroup(m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y, Layers()->GameGroup());
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();

	// render cursor
	RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[m_pClient->m_Snap.m_pLocalCharacter->m_Weapon%NUM_WEAPONS].m_pSpriteCursor);
	float CursorSize = 64;
	vec2 Pos = m_pClient->m_pControls->m_TargetPos;
	RenderTools()->DrawSprite(Pos.x, Pos.y, CursorSize);
	Graphics()->QuadsEnd();

	// render hit indicator
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();

	int acc = 100;
	float t = acc / 0.75f;

	//TODO
	float a = (float)clamp(acc-(int)((time_get()-m_LastHit)/(time_freq()/t/*zeit*/)), 0, acc)/acc;
	//int64 t = m_LastHit-time_get() + time_freq();
	//if(t<=0)
	//	a = 0;
	//else
	//	a = 100.0f/(m_LastHit-time_get() + time_freq());

	//	dbg_msg(0, "%f", a);

	Graphics()->SetColor(1.0f, 0.2f, 0.2f, a);


	IGraphics::CQuadItem Array[4];
	CursorSize = 40;
	Array[0] = IGraphics::CQuadItem(Pos.x-1, Pos.y-CursorSize, 3, 10);
	Array[1] = IGraphics::CQuadItem(Pos.x-CursorSize, Pos.y-1, 10, 3);
	Array[2] = IGraphics::CQuadItem(Pos.x-1, Pos.y+CursorSize-10, 3, 10);
	Array[3] = IGraphics::CQuadItem(Pos.x+CursorSize-10, Pos.y-1, 10, 3);
	Graphics()->QuadsDrawTL(Array, 4);
	Graphics()->QuadsEnd();
}

void CHud::RenderHealthAndAmmo(const CNetObj_Character *pCharacter)
{
	if(!pCharacter)
		return;

	//mapscreen_to_group(gacenter_x, center_y, layers_game_group());

	float x = 5;
	float y = 5;
	float w = 12; // orig: 12;

	// render ammo count
	// render gui stuff

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);

	Graphics()->QuadsBegin();

	// if weaponstage is active, put a "glow" around the stage ammo
	RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[pCharacter->m_Weapon%NUM_WEAPONS].m_pSpriteProj);
	IGraphics::CQuadItem Array[10];
	int i;
	for (i = 0; i < min(pCharacter->m_AmmoCount, 10); i++)
		Array[i] = IGraphics::CQuadItem(x+i*12,y+24,10,10);
	Graphics()->QuadsDrawTL(Array, i);
	Graphics()->QuadsEnd();

	Graphics()->QuadsBegin();
	int h = 0;

	// render health
	RenderTools()->SelectSprite(SPRITE_HEALTH_FULL);
	for(; h < min(pCharacter->m_Health, 10); h++)
		Array[h] = IGraphics::CQuadItem(x+h*w,y,10,10);
	Graphics()->QuadsDrawTL(Array, h);

	i = 0;
	RenderTools()->SelectSprite(SPRITE_HEALTH_EMPTY);
	for(; h < 10; h++)
		Array[i++] = IGraphics::CQuadItem(x+h*w,y,10,10);
	Graphics()->QuadsDrawTL(Array, i);

	// render armor meter
	h = 0;
	RenderTools()->SelectSprite(SPRITE_ARMOR_FULL);
	for(; h < min(pCharacter->m_Armor, 10); h++)
		Array[h] = IGraphics::CQuadItem(x+h*w,y+w,10,10);
	Graphics()->QuadsDrawTL(Array, h);

	i = 0;
	RenderTools()->SelectSprite(SPRITE_ARMOR_EMPTY);
	for(; h < 10; h++)
		Array[i++] = IGraphics::CQuadItem(x+h*w,y+w,10,10);
	Graphics()->QuadsDrawTL(Array, i);
	Graphics()->QuadsEnd();
}

void CHud::RenderSXHealthAndAmmo(const CNetObj_Character *pCharacter)
{
	if(!pCharacter)
		return;

	float x = 5;
	float y = 5;
	float w = 9;

	// render background
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.5f);
	RenderTools()->DrawRoundRectExt(0, y-2, 10*12, 26, 4.0f, CUI::CORNER_R); // health & armor
	for(int i = 0; i < NUM_WEAPONS; i++)
	{
		if(i == pCharacter->m_Weapon%NUM_WEAPONS)
			Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.25f);
		else
			Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.6f);
		if(i == 0 || i == NUM_WEAPONS-1)
			RenderTools()->DrawRoundRectExt(x+i*18, y+24, 18, 12, 4.0f, i == 0 ? CUI::CORNER_BL : CUI::CORNER_BR);
		else
			RenderTools()->DrawRoundRectExt(x+i*18, y+24, 18, 12, 0.0f, 0xf);
	}
	Graphics()->QuadsEnd();

	// render weapon
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();
	for(int i = 0; i < NUM_WEAPONS; i++)
	{
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i%NUM_WEAPONS].m_pSpriteBody);
		RenderTools()->DrawSprite(x+9+i*18, y+6+24, 18);
	}
	Graphics()->QuadsEnd();
	

	// render ammo count
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[pCharacter->m_Weapon%NUM_WEAPONS].m_pSpriteProj);
	IGraphics::CQuadItem Array[10];
	int i;
	for (i = 0; i < min(pCharacter->m_AmmoCount, 10); i++)
		Array[i] = IGraphics::CQuadItem(x+i*10.8f,y+36,10,10);
	Graphics()->QuadsDrawTL(Array, i);
	Graphics()->QuadsEnd();

	Graphics()->QuadsBegin();
	int h = 0;

	// render health
	RenderTools()->SelectSprite(SPRITE_HEALTH_FULL);
	for(; h < min(pCharacter->m_Health, 10); h++)
		Array[h] = IGraphics::CQuadItem(x+(h+2)*w,y,10,10);
	Graphics()->QuadsDrawTL(Array, h);

	i = 0;
	RenderTools()->SelectSprite(SPRITE_ARMOR_EMPTY/*SPRITE_HEALTH_EMPTY*/);
	for(; h < 10; h++)
		Array[i++] = IGraphics::CQuadItem(x+(h+2)*w,y,10,10);
	Graphics()->QuadsDrawTL(Array, i);

	// render armor meter
	h = 0;
	RenderTools()->SelectSprite(SPRITE_ARMOR_FULL);
	for(; h < min(pCharacter->m_Armor, 10); h++)
		Array[h] = IGraphics::CQuadItem(x+(h+2)*w,y+12,10,10);
	Graphics()->QuadsDrawTL(Array, h);

	i = 0;
	RenderTools()->SelectSprite(SPRITE_ARMOR_EMPTY);
	for(; h < 10; h++)
		Array[i++] = IGraphics::CQuadItem(x+(h+2)*w,y+12,10,10);
	Graphics()->QuadsDrawTL(Array, i);
	Graphics()->QuadsEnd();

	// render some labels
	char Buf[32];
	str_format(Buf, sizeof(Buf), "HP");
	TextRender()->TextColor(1.0f, 0.5f, 0.5f, 1.0f);	
	TextRender()->Text(0, x+2.5f, 3.5f, 10.0f, Buf, -1);
	str_format(Buf, sizeof(Buf), "AP");
	TextRender()->TextColor(0.9f, 0.6f, 0.15f, 1.0f);	
	TextRender()->Text(0, x+2.5f, 15.5f, 10.0f, Buf, -1);
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CHud::RenderSpectatorHud()
{
	// draw the box
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.4f);
	RenderTools()->DrawRoundRectExt(m_Width-180.0f, m_Height-15.0f, 180.0f, 15.0f, 5.0f, CUI::CORNER_TL);
	Graphics()->QuadsEnd();

	// draw the text
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Spectate"), m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW ?
		m_pClient->m_aClients[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID].m_aName : Localize("Free-View"));
	TextRender()->Text(0, m_Width-174.0f, m_Height-13.0f, 8.0f, aBuf, -1);
}

void CHud::OnRender()
{
	if(!m_pClient->m_Snap.m_pGameInfoObj)
		return;

	m_Width = 300.0f*Graphics()->ScreenAspect();
	m_Height = 300.0f;
	Graphics()->MapScreen(0.0f, 0.0f, m_Width, m_Height);

	if(g_Config.m_ClShowhud)
	{
		if(m_pClient->m_Snap.m_pLocalCharacter && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
		{
			//RenderHealthAndAmmo(m_pClient->m_Snap.m_pLocalCharacter);
			RenderSXHealthAndAmmo(m_pClient->m_Snap.m_pLocalCharacter);
		}
		else if(m_pClient->m_Snap.m_SpecInfo.m_Active)
		{
			if(m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW)
				RenderHealthAndAmmo(&m_pClient->m_Snap.m_aCharacters[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID].m_Cur);
			RenderSpectatorHud();
		}

		RenderGameTimer();
		RenderPauseNotification();
		RenderSuddenDeath();
		RenderScoreHud();
		//RenderSXScoreHud();
		RenderSXRadar();
		RenderSXPing();
		RenderWarmupTimer();
		RenderFps();
		if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
			RenderConnectionWarning();
		RenderTeambalanceWarning();
		RenderVoting();
	}
	RenderCursor();
}
