/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/math.h>

#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/client/components/sounds.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <game/localization.h>

#include "binds.h"
#include "countryflags.h"
#include "menus.h"
#include "skins.h"


void CMenus::ConKeyShortcut(IConsole::IResult *pResult, void *pUserData)
{
	CMenus *pSelf = (CMenus *)pUserData;
	if(pSelf->Client()->State() == IClient::STATE_ONLINE)
	{
		pSelf->m_QAActive = pResult->GetInteger(0) != 0;
	}
}

void CMenus::OnConsoleInit()
{
	Console()->Register("+shortcut", "", CFGFLAG_CLIENT, ConKeyShortcut, this, "Open shortcut selector");
}

void CMenus::RenderQADummy(CUIRect MainView)
{
#define DUMMY_HELPER(part) g_Config.m_XDummy##part
#define DUMMY(part ,id) id == 1 ? DUMMY_HELPER(part ## 1) : id == 2 ? DUMMY_HELPER(part ## 2) : DUMMY_HELPER(part ## 3)
#define FAKE_HELPER(part) g_Config.m_XFake##part
#define FAKE(part ,id) id == 1 ? FAKE_HELPER(part ## 1) : id == 2 ? FAKE_HELPER(part ## 2) : id == 3 ? FAKE_HELPER(part ## 3) : id == 4 ? FAKE_HELPER(part ## 4) : FAKE_HELPER(part ## 5) 

	const int NumDummys = 3 + 1;
	const int NumIdentities = 5 + 1; // + original name
	CUIRect Button, Label, Temp;
	static float s_Scrollbar = 0;
	static float s_ScrollValue = 0;
	static int s_Page = 0;
	static int s_Dummy[NumDummys];
	static int s_Identity[NumIdentities];
	static int SelectedDummy = 0, SelectedIdentity = g_Config.m_XFakeId;
	

	// main form
	
	RenderTools()->DrawUIRect(&MainView, vec4(0.0f, 0.0f, 0.0f, 0.5f), CUI::CORNER_B, 10.0f);

	// tabs
	MainView.HSplitTop(24.0f, &Button, &Temp);
	Button.VSplitMid(&Button, &Label);
	static int s_TabDummy = 0;
	if(DoButton_MenuTab(&s_TabDummy, "Dummy", s_Page == 0, &Button, 0))
		s_Page = 0;
	static int s_TabIdentity = 0;
	if(DoButton_MenuTab(&s_TabIdentity, "Identity", s_Page == 1, &Label, 0))
		s_Page = 1;

	// tee selector
	Temp.HSplitTop(68.0f, &Button, &Temp);
	UI()->ClipEnable(&Button);
	Button.HMargin(5.0f, &Button);
	Button.VSplitLeft(7.5f, 0, &Button);

	int Offset = 30.0f + 80 * ((s_Page ? NumIdentities : NumDummys) - 4);
	if(Offset > 0)
		Button.x -= Offset * s_ScrollValue;
	if(s_Page == 0)
	{
		for(int i = 0; i < NumDummys; i++)
		{
			Button.VSplitLeft(80.0f, &Label, &Button);
			if(DoButton_Menu(&s_Dummy[i], "", 0, &Label))
				SelectedDummy = i;

			const CSkins::CSkin *pSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(DUMMY(Skin, i)));
			CTeeRenderInfo SkinInfo;
			if(i)
			{
				if(DUMMY(UseCustomColor, i))
				{
					SkinInfo.m_Texture = pSkin->m_ColorTexture;
					SkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(DUMMY(ColorBody, i));
					SkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(DUMMY(ColorFeet, i));
				}
				else
				{
					SkinInfo.m_Texture = pSkin->m_OrgTexture;
					SkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
					SkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}
			else
			{
				pSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(g_Config.m_PlayerSkin));
				if(g_Config.m_PlayerUseCustomColor)
				{
					SkinInfo.m_Texture = pSkin->m_ColorTexture;
					SkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorBody);
					SkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorFeet);
				}
				else
				{
					SkinInfo.m_Texture = pSkin->m_OrgTexture;
					SkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
					SkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}
			SkinInfo.m_Size = 50.0f*UI()->Scale();
			RenderTools()->RenderTee(CAnimState::GetIdle(), &SkinInfo, 0, vec2(1, 0), vec2(Label.x + Label.w * 0.5f, Label.y + Label.h * 0.5f));

			Label.HSplitBottom(10.0f, 0, &Label);
			RenderTools()->DrawUIRect(&Label, SelectedDummy == i ? vec4(0.2f, 1.0f, 0.2f, 0.5f) : vec4(0.0f, 0.0f, 0.0f, 0.5f), CUI::CORNER_B, 4.0f);
			UI()->DoLabelScaled(&Label, i ? DUMMY(Name, i) : g_Config.m_PlayerName, 8.0f, 0);
			Button.VSplitLeft(15.0f, 0, &Button);
		}
	}
	else
	{
		for(int i = 1; i < NumIdentities; i++)
		{
			Button.VSplitLeft(80.0f, &Label, &Button);
			if(DoButton_Menu(&s_Identity[i], "", 0, &Label))
				SelectedIdentity = i;
			const CSkins::CSkin *pSkin = NULL;
			CTeeRenderInfo SkinInfo;

			if(i)
			{
				pSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(FAKE(Skin, i)));
				if(FAKE(UseCustomColor, i))
				{
					SkinInfo.m_Texture = pSkin->m_ColorTexture;
					SkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(FAKE(ColorBody, i));
					SkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(FAKE(ColorFeet, i));
				}
				else
				{
					SkinInfo.m_Texture = pSkin->m_OrgTexture;
					SkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
					SkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}
			
			SkinInfo.m_Size = 50.0f*UI()->Scale();
			RenderTools()->RenderTee(CAnimState::GetIdle(), &SkinInfo, 0, vec2(1, 0), vec2(Label.x + Label.w * 0.5f, Label.y + Label.h * 0.5f));

			Label.HSplitBottom(10.0f, 0, &Label);
			RenderTools()->DrawUIRect(&Label, SelectedIdentity == i ? vec4(0.2f, 1.0f, 0.2f, 0.5f) : vec4(0.0f, 0.0f, 0.0f, 0.5f), CUI::CORNER_B, 4.0f);
			UI()->DoLabelScaled(&Label, i ? FAKE(Name, i) : g_Config.m_PlayerName, 8.0f, 0);
			Button.VSplitLeft(15.0f, 0, &Button);
		}
	}
	UI()->ClipDisable();

	// scrollbar
	Temp.HSplitTop(16.0f, &Button, &Temp);
	Button.VMargin(4.0f, &Button);
	s_ScrollValue = DoScrollbarH(&s_Scrollbar, &Button, s_ScrollValue);

	// Buttons
	Temp.HMargin(4.0f, &Temp);
	Temp.VMargin(10.0f, &Temp);

	if(s_Page == 0)
	{
		static int s_ButtonConnect = 0;
		static int s_ButtonCenter = 0;
		static int s_ButtonInput = 0;
		int Id = SelectedDummy - 1;
		Temp.HMargin(12.0f, &Temp);
		Temp.VSplitLeft(100.0f, &Button, &Temp);

		if(SelectedDummy)
		{
			if(Client()->GetDummyFlags(SelectedDummy-1)&IClient::DUMMYFLAG_ACTIVE == 1)
			{
				if(DoButton_Menu(&s_ButtonConnect, "Disconnect", 0, &Button, NULL, vec4(0.7f, 1, 0.7f, 0.5f)))
				{
					Client()->DisconnectDummy(Id);
				}
			}
			else
			{
				if(DoButton_Menu(&s_ButtonConnect, "Connect", 0, &Button))
				{
					Client()->ConnectDummy(Id);
				}
			}
		}
		Temp.VSplitLeft(30.0f, 0, &Temp);
		Temp.VSplitLeft(100.0f, &Button, &Temp);
		if(DoButton_Menu(&s_ButtonCenter, "Center", 0, &Button, NULL, Client()->GetCentralDummy() == SelectedDummy-1? vec4(0.7f, 1.0f, 0.7f, 0.5f) : vec4(1.0f, 1.0f, 1.0f, 0.5f)))
		{
			Client()->SetCentralDummy(Id);
		}
		Temp.VSplitLeft(30.0f, 0, &Temp);
		Temp.VSplitLeft(100.0f, &Button, &Temp);
		if(DoButton_Menu(&s_ButtonInput, "Input", 0, &Button, NULL, Client()->GetDummyMoving(SelectedDummy-1)? vec4(0.7f, 1.0f, 0.7f, 0.5f) : vec4(1.0f, 1.0f, 1.0f, 0.5f)))
		{
			Client()->ToggleInputDummy(Id);
		}
	}
	else
	{
		static int s_ButtonUse = 0;
		Temp.HMargin(12.0f, &Button);
		if(DoButton_Menu(&s_ButtonUse, "Use Identity", 0, &Button))
		{
			str_format(g_Config.m_PlayerName, sizeof(g_Config.m_PlayerName), FAKE(Name, SelectedIdentity));
			str_format(g_Config.m_PlayerClan, sizeof(g_Config.m_PlayerClan), FAKE(Clan, SelectedIdentity));
			str_format(g_Config.m_PlayerSkin, sizeof(g_Config.m_PlayerSkin), FAKE(Skin, SelectedIdentity));
			g_Config.m_PlayerUseCustomColor = FAKE(UseCustomColor, SelectedIdentity);
			g_Config.m_PlayerColorBody = FAKE(ColorBody, SelectedIdentity);
			g_Config.m_PlayerColorFeet = FAKE(ColorFeet, SelectedIdentity);
			m_pClient->SendInfo(false);
		}

	}


#undef DUMMY_HELPER
#undef DUMMY
}

void CMenus::RenderQAMark(CUIRect MainView)
{
	MainView.w = MainView.h = 64.0f;
	RenderTools()->DrawUIRect(&MainView, vec4(0.2f, 1.0f, 0.2f, 0.5f) , CUI::CORNER_ALL, 4.0f);

}


void CMenus::RenderQA(CUIRect MainView)
{
	if(!m_QAActive)
	{
		m_QAWasActive = false;
		return;
	}

	/*if(m_pClient->m_Snap.m_SpecInfo.m_Active)
	{
		m_QAActive = false;
		m_QAWasActive = false;
		return;
	}*/

	m_QAWasActive = true;

	CUIRect Screen = *UI()->Screen();

	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	Graphics()->BlendNormal();

	CUIRect r;
	MainView.HSplitTop(160.0f, &r, 0);
	r.w = 380.0f;
	r.x = MainView.w / 2 - r.w / 2;
	RenderQADummy(r);

	if(!UI()->MouseInside(&r))
	{

	}





	//Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	//Graphics()->QuadsBegin();
	//Graphics()->SetColor(1, 1, 1, 1);
	//IGraphics::CQuadItem QuadItem(m_SelectorMouse.x/*+Screen.w/2*/, m_SelectorMouse.y/*+Screen.h/2*/, 24, 24);
	//Graphics()->QuadsDrawTL(&QuadItem, 1);
	//Graphics()->QuadsEnd();
}
