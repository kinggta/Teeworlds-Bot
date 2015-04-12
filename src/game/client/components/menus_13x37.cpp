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
#include "teefiles.h"

void CMenus::Render13x37Identity(CUIRect MainView)
{

	// render background
	CUIRect Temp, TabBar, Button, Label, View;
	static int Page = 0;
		
	const int Test = 64; //Todo: remove this and code it the right way
	int numID = m_pClient->m_pTeeFiles->Num();

	MainView.VSplitLeft(240.0f, &TabBar, &MainView);
	TabBar.VSplitRight(2.0f, &TabBar, &Button);
	RenderTools()->DrawUIRect(&Button, vec4(0.0f, 0.8f, 0.6f, 0.5f), 0, 0);
	
	for(int i = 0; i < numID; i++)
	{
		CTeeFiles::CTee *pTee = m_pClient->m_pTeeFiles->Get(i);
		TabBar.HSplitTop(24.0f, &Button, &TabBar);
		if(DoButton_MenuTab(pTee->m_aName, "", Page == i, &Button, 0))
			Page = i;

		Button.VSplitRight(Button.h, 0, &Temp);
		Temp.Margin(4.0f, &Temp);
		if(DoButton_Menu(pTee, Localize("X"), 0, &Temp, "", vec4(0.75f, 0.25f, 0.25f, 0.75f)))
			m_pClient->m_pTeeFiles->Remove(i);

		Button.HSplitTop(Button.h*0.25f, 0, &Label);

		const CSkins::CSkin *pOwnSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(pTee->m_aSkin));
		CTeeRenderInfo OwnSkinInfo;
		if(pTee->m_UseCustomColor)
		{
			OwnSkinInfo.m_Texture = pOwnSkin->m_ColorTexture;
			OwnSkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(pTee->m_ColorBody);
			OwnSkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(pTee->m_ColorFeet);
		}
		else
		{
			OwnSkinInfo.m_Texture = pOwnSkin->m_OrgTexture;
			OwnSkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			OwnSkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		OwnSkinInfo.m_Size = 26.0f*UI()->Scale();
		RenderTools()->RenderTee(CAnimState::GetIdle(), &OwnSkinInfo, 0, vec2(1, 0), vec2(Button.x + OwnSkinInfo.m_Size, Button.y + Button.h *0.6f));
		Button.HMargin(2.0f, &Button);
		Button.HSplitBottom(16.0f, 0, &Button);
		UI()->DoLabelScaled(&Button, pTee->m_aName, 14.0f, 0);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	
	static int s_ButtonAdd = 0;

	TabBar.HSplitTop(24.0f, &Button, &TabBar);
	Button.VSplitRight(Button.h, 0, &Temp);
	Temp.Margin(4.0f, &Temp);
	if(DoButton_Menu(&s_ButtonAdd, Localize("+"), 0, &Temp, "", vec4(0.25f, 0.75f, 0.25f, 0.75f)))
	{
		CTeeFiles::CTee Tee;
		mem_zero(&Tee, sizeof(Tee));
		str_format(Tee.m_aName, sizeof(Tee.m_aName), "nameless tee");
		str_format(Tee.m_aSkin, sizeof(Tee.m_aSkin), "default");
		m_pClient->m_pTeeFiles->Add(Tee);
	}

	MainView.Margin(10.0f, &MainView);

	MainView.Margin(10.0f, &MainView);
	MainView.HSplitTop(10.0f, 0, &View);

	View.HSplitTop(20.0f, 0, &View);
	View.HSplitTop(20.0f, &Button, &View);
	Button.VSplitLeft(230.0f, &Button, 0);

	static int s_UseID = 0; //TODO ändern
	CTeeFiles::CTee *pTee = m_pClient->m_pTeeFiles->Get(Page);
	if(!m_pClient->m_pTeeFiles->Num() || !pTee)
		return;

	if(DoButton_Menu(&s_UseID, Localize("Use ID"), 0, &Button))
	{
		str_format(g_Config.m_PlayerName, sizeof(g_Config.m_PlayerName), pTee->m_aName);
		str_format(g_Config.m_PlayerClan, sizeof(g_Config.m_PlayerClan), pTee->m_aClan);
		str_format(g_Config.m_PlayerSkin, sizeof(g_Config.m_PlayerSkin), pTee->m_aSkin);
		g_Config.m_PlayerUseCustomColor = pTee->m_UseCustomColor;
		g_Config.m_PlayerColorBody = pTee->m_ColorBody;
		g_Config.m_PlayerColorFeet = pTee->m_ColorFeet;
		m_NeedSendinfo = true;
	}

	// skin info
	const CSkins::CSkin *pOwnSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(pTee->m_aSkin));
	CTeeRenderInfo OwnSkinInfo;
	if(pTee->m_UseCustomColor)
	{
		OwnSkinInfo.m_Texture = pOwnSkin->m_ColorTexture;
		OwnSkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(pTee->m_ColorBody);
		OwnSkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(pTee->m_ColorFeet);
	}
	else
	{
		OwnSkinInfo.m_Texture = pOwnSkin->m_OrgTexture;
		OwnSkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		OwnSkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	OwnSkinInfo.m_Size = 50.0f*UI()->Scale();

	View.HSplitTop(20.0f, &Label, &View);
	Label.VSplitLeft(230.0f, &Label, 0);
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Your skin"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0f, -1);

	View.HSplitTop(50.0f, &Label, &View);
	Label.VSplitLeft(230.0f, &Label, 0);
	RenderTools()->DrawUIRect(&Label, vec4(1.0f, 1.0f, 1.0f, 0.25f), CUI::CORNER_ALL, 10.0f);
	RenderTools()->RenderTee(CAnimState::GetIdle(), &OwnSkinInfo, 0, vec2(1, 0), vec2(Label.x+30.0f, Label.y+28.0f));
	Label.HSplitTop(15.0f, 0, &Label);;
	Label.VSplitLeft(70.0f, 0, &Label);
	UI()->DoLabelScaled(&Label, pTee->m_aSkin, 14.0f, -1, 150.0f);

	// custom colour selector
	View.HSplitTop(20.0f, 0, &View);
	View.HSplitTop(20.0f, &Button, &View);
	Button.VSplitLeft(230.0f, &Button, 0);
	if(DoButton_CheckBox(&pTee->m_UseCustomColor, Localize("Custom colors"), pTee->m_UseCustomColor, &Button))
	{
		pTee->m_UseCustomColor ^= 1;
		m_NeedSendinfo = true;
	}

	View.HSplitTop(5.0f, 0, &View);
	//View.HSplitTop(82.5f, &Label, &View);
	if(pTee->m_UseCustomColor)
	{
		CUIRect aRects[2];
		Label.VSplitMid(&aRects[0], &aRects[1]);
		View.VSplitMid(&aRects[0], 0);
		aRects[0].HSplitMid(&aRects[0], &aRects[1]);

		aRects[0].VSplitRight(10.0f, &aRects[0], 0);
		aRects[1].VSplitRight(10.0f, &aRects[1], 0);

		int *paColors[2];
		paColors[0] = &pTee->m_ColorBody;
		paColors[1] = &pTee->m_ColorFeet;

		const char *paParts[] = {
			Localize("Body"),
			Localize("Feet")};
		const char *paLabels[] = {
			Localize("Hue"),
			Localize("Sat."),
			Localize("Lht.")};
		static int s_aColorSlider[2][3] = { { 0 } };

		for(int i = 0; i < 2; i++)
		{
			aRects[i].HSplitTop(20.0f, &Label, &aRects[i]);
			UI()->DoLabelScaled(&Label, paParts[i], 14.0f, -1);
			aRects[i].VSplitLeft(20.0f, 0, &aRects[i]);
			aRects[i].HSplitTop(2.5f, 0, &aRects[i]);

			int PrevColor = *paColors[i];
			int Color = 0;
			for(int s = 0; s < 3; s++)
			{
				aRects[i].HSplitTop(20.0f, &Label, &aRects[i]);
				Label.VSplitLeft(100.0f, &Label, &Button);
				Button.HMargin(2.0f, &Button);

				float k = ((PrevColor>>((2-s)*8))&0xff) / 255.0f;
				k = DoScrollbarH(&s_aColorSlider[i][s], &Button, k);
				Color <<= 8;
				Color += clamp((int)(k*255), 0, 255);
				UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
			}

			if(PrevColor != Color)
				m_NeedSendinfo = true;

			*paColors[i] = Color;
		}
	}

	// skin selector
	MainView.VSplitMid(0, &MainView);

	// player name
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(150.0f, &Button, 0);
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Name"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0, -1);
	static float s_OffsetName = 0.0f;
	if(DoEditBox(&pTee->m_aName, &Button, pTee->m_aName, sizeof(g_Config.m_PlayerName), 14.0f, &s_OffsetName))
		m_NeedSendinfo = true;

	// player clan
	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(150.0f, &Button, 0);
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Clan"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0, -1);
	static float s_OffsetClan = 0.0f;
	if(DoEditBox(&pTee->m_aClan, &Button, pTee->m_aClan, sizeof(g_Config.m_PlayerClan), 14.0f, &s_OffsetClan))
		m_NeedSendinfo = true;

	MainView.HSplitTop(10.0f, 0, &MainView);
	static bool s_InitSkinlist = false;
	static sorted_array<const CSkins::CSkin *> s_paSkinList;
	static float s_ScrollValue = {0.0f};

	if(!s_InitSkinlist)
	{
		s_paSkinList.clear();
		for(int i = 0; i < m_pClient->m_pSkins->Num(); ++i)
		{
			const CSkins::CSkin *s = m_pClient->m_pSkins->Get(i);
			// no special skins
			if(s->m_aName[0] == 'x' && s->m_aName[1] == '_')
				continue;
			s_paSkinList.add(s);
		}
		s_InitSkinlist = true;
	}

	int OldSelected = -1;
	UiDoListboxStart(&s_InitSkinlist, &MainView, 50.0f, Localize("Skins"), "", s_paSkinList.size(), 4, OldSelected, s_ScrollValue);

	for(int i = 0; i < s_paSkinList.size(); i++)
	{
		
		const CSkins::CSkin *s = s_paSkinList[i];

		if(s == 0)
			continue;

		if(str_comp(s->m_aName, pTee->m_aSkin) == 0)
			OldSelected = i;

		CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[i], OldSelected == i);
		if(Item.m_Visible)
		{
			CTeeRenderInfo Info;
			if(pTee->m_UseCustomColor)
			{
				Info.m_Texture = s->m_ColorTexture;
				Info.m_ColorBody = m_pClient->m_pSkins->GetColorV4(pTee->m_ColorBody);
				Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(pTee->m_ColorFeet);
			}
			else
			{
				Info.m_Texture = s->m_OrgTexture;
				Info.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				Info.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			Info.m_Size = UI()->Scale()*50.0f;
			Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
			RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, 0, vec2(1.0f, 0.0f), vec2(Item.m_Rect.x+Item.m_Rect.w/2, Item.m_Rect.y+Item.m_Rect.h/2));

			if(g_Config.m_Debug)
			{
				vec3 BloodColor = pTee->m_UseCustomColor ? m_pClient->m_pSkins->GetColorV3(pTee->m_ColorBody) : s->m_BloodColor;
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				Graphics()->SetColor(BloodColor.r, BloodColor.g, BloodColor.b, 1.0f);
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x, Item.m_Rect.y, 12.0f, 12.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
	if(OldSelected != NewSelected)
	{
		str_format(pTee->m_aSkin, sizeof(s_paSkinList[NewSelected]->m_aName), s_paSkinList[NewSelected]->m_aName);
		m_NeedSendinfo = true;
	}
}

void CMenus::Render13x37Econ(CUIRect MainView)
{
	//CUIRect Button, Label, Console;
	//char aBuf[128];
	//static char aCmd[512];

	//MainView.Margin(10.0f, &MainView);

	//// server address
	//MainView.HSplitTop(20.0f, &Button, &MainView);
	//Button.VSplitLeft(80.0f, &Label, &Button);
	//Button.VSplitLeft(150.0f, &Button, 0);
	//
	//str_format(aBuf, sizeof(aBuf), "%s:", Localize("Address"));
	//UI()->DoLabelScaled(&Label, aBuf, 14.0, -1);
	//static float s_OffsetAddress = 0.0f;
	//DoEditBox(g_Config.m_XBindaddr, &Button, g_Config.m_XBindaddr, sizeof(g_Config.m_XBindaddr), 14.0f, &s_OffsetAddress);
	//
	//Button.x += 160.0f;
	//static int s_ConnectButton = 0;
	//if(DoButton_Menu((void*)&s_ConnectButton, Localize("Connect"), 0, &Button))
	//	EconClient()->Connect(g_Config.m_XBindaddr);

	////- recieved strings
	//MainView.HSplitTop(10.0f, 0, &MainView);
	//MainView.HSplitBottom(30.0f, &Console, &MainView);
	//Console.VMargin(2.5f, &Console);
	//RenderTools()->DrawUIRect(&Console, vec4(0,0,0,0.5f),  CUI::CORNER_ALL, 10.0f);
	//Console.VMargin(5.0f, &Console);

	//EconClient()->Update();

	////UI()->ClipEnable(&Console);//dont let the text esacpe

	//static int s_ScrollBar = 0;
	//static float s_ScrollValue = 0.0f;

	//int Start = clamp((int)s_ScrollValue, 0, MAX_CON_LINES-24);
	//int Stop = clamp(Start+24, 0, MAX_CON_LINES);

	//Console.VSplitRight(15.0f, &Console, &Button);
	//Button.HMargin(5.0f, &Button);
	//float tmp = EconClient()->GetLineCount()-24;
	//if(tmp>0)
	//	s_ScrollValue = (float)DoScrollbarV(&s_ScrollValue, &Button, s_ScrollValue/tmp)*tmp;
	//
	//

	//for(int i = Start; i < Stop; i++)
	//{
	//	UI()->DoLabelScaled(&Console, EconClient()->m_aLines[i], 10.37f, -1);
	//	Console.HSplitTop(12.0f, &Label, &Console);
	//}


	//if(Input()->KeyPresses(KEY_MOUSE_WHEEL_UP) && tmp>0 && s_ScrollValue >= 1.0f)
	//	s_ScrollValue -= 1.0f;
	//if(Input()->KeyPresses(KEY_MOUSE_WHEEL_DOWN) && tmp>0&& s_ScrollValue <= tmp-1.0f)
	//	s_ScrollValue += 1.0f;



	////UI()->ClipDisable();

	//// command to send
	//MainView.HSplitTop(10.0f, 0, &MainView);
	//MainView.VSplitRight(150.0f, &Label, &Button);
	//static float s_OffsetCommand = 0.0f;
	//DoEditBox(&aCmd, &Label, aCmd, sizeof(aCmd), 14.0f, &s_OffsetCommand);

	//Button.VSplitLeft(10.0f, 0, &Button);
	//static int s_SendButton = 0;
	//if(DoButton_Menu((void*)&s_SendButton, Localize("Send"), 0, &Button) || m_EnterPressed)
	//{
	//	EconClient()->Send(aCmd, str_length(aCmd));
	//	mem_zero(&aCmd, sizeof(aCmd));
	//}
}

void CMenus::Render13x37Extras(CUIRect MainView)
{
}
void CMenus::Render13x37Esp(CUIRect MainView)
{
}
void CMenus::Render13x37Tee(CUIRect MainView)
{

}

void CMenus::Render13x37Dummy(CUIRect MainView)
{
#define DUMMY_HELPER(part) g_Config.m_XDummy##part
#define DUMMY(part ,id) id == 0 ? DUMMY_HELPER(part ## 1) : id == 1 ? DUMMY_HELPER(part ## 2) : DUMMY_HELPER(part ## 3)

	// render background
	CUIRect Temp, TabBar, Button, Label, View;
	static int Page = 0;
	const char *aTabs[] = {
		DUMMY(Name, 0),
		DUMMY(Name, 1),
		DUMMY(Name, 2), };
	const int numID = (int)(sizeof(aTabs) / sizeof(*aTabs));

	MainView.VSplitLeft(240.0f, &TabBar, &MainView);
	TabBar.VSplitRight(2.0f, &TabBar, &Button);
	RenderTools()->DrawUIRect(&Button, vec4(0.0f, 0.8f, 0.6f, 0.5f), 0, 0);

	for (int i = 0; i < numID; i++)
	{
		TabBar.HSplitTop(24.0f, &Button, &TabBar);
		if (DoButton_MenuTab(aTabs[i], "", Page == i, &Button, 0))
			Page = i;

		const CSkins::CSkin *pOwnSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(DUMMY(Skin, i)));
		CTeeRenderInfo OwnSkinInfo;
		if (DUMMY(UseCustomColor, i))
		{
			OwnSkinInfo.m_Texture = pOwnSkin->m_ColorTexture;
			OwnSkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(DUMMY(ColorBody, i));
			OwnSkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(DUMMY(ColorFeet, i));
		}
		else
		{
			OwnSkinInfo.m_Texture = pOwnSkin->m_OrgTexture;
			OwnSkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			OwnSkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		OwnSkinInfo.m_Size = 26.0f*UI()->Scale();
		RenderTools()->RenderTee(CAnimState::GetIdle(), &OwnSkinInfo, 0, vec2(1, 0), vec2(Button.x + Button.w - OwnSkinInfo.m_Size, Button.y + Button.h *0.6f));

		Button.HMargin(2.0f, &Button);
		Button.HSplitBottom(16.0f, 0, &Button);
		UI()->DoLabelScaled(&Button, aTabs[i], 14.0f, 0);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	MainView.Margin(10.0f, &MainView);

	int *pUseCustomColor = NULL, *pColorBody = NULL, *pColorFeet = NULL;
	char *pName, *pClan, *pSkin;

#undef DUMMY_HELPER
#define DUMMY_HELPER(part) &g_Config.m_XDummy##part
	pUseCustomColor = DUMMY(UseCustomColor, Page);
	pColorBody = DUMMY(ColorBody, Page);
	pColorFeet = DUMMY(ColorFeet, Page);
#undef DUMMY_HELPER
#define DUMMY_HELPER(part) &g_Config.m_XDummy##part[0]
	pName = DUMMY(Name, Page);
	pClan = DUMMY(Clan, Page);
	pSkin = DUMMY(Skin, Page);

#undef DUMMY
#undef DUMMY_HELPER

	MainView.Margin(10.0f, &MainView);
	MainView.HSplitTop(10.0f, 0, &View);

	View.HSplitTop(20.0f, 0, &View);
	View.HSplitTop(20.0f, &Button, &View);
	Button.VSplitLeft(230.0f, &Button, 0);

	static int s_ConnectDummy = 0;
	if (DoButton_Menu(&s_ConnectDummy, Client()->GetDummyFlags(Page)&IClient::DUMMYFLAG_ACTIVE ? Localize("Disconnect Dummy") : Localize("Connect Dummy"), s_ConnectDummy, &Button))
	{
		//s_ConnectDummy ^= 1;
		if (Client()->GetDummyFlags(Page)&IClient::DUMMYFLAG_ACTIVE == 1)
			Client()->DisconnectDummy(Page);
		else
			Client()->ConnectDummy(Page);
	}



	// skin info
	const CSkins::CSkin *pOwnSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(pSkin));
	CTeeRenderInfo OwnSkinInfo;
	if (*pUseCustomColor)
	{
		OwnSkinInfo.m_Texture = pOwnSkin->m_ColorTexture;
		OwnSkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(*pColorBody);
		OwnSkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(*pColorFeet);
	}
	else
	{
		OwnSkinInfo.m_Texture = pOwnSkin->m_OrgTexture;
		OwnSkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		OwnSkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	OwnSkinInfo.m_Size = 50.0f*UI()->Scale();

	View.HSplitTop(20.0f, &Label, &View);
	Label.VSplitLeft(230.0f, &Label, 0);
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Your skin"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0f, -1);

	View.HSplitTop(50.0f, &Label, &View);
	Label.VSplitLeft(230.0f, &Label, 0);
	RenderTools()->DrawUIRect(&Label, vec4(1.0f, 1.0f, 1.0f, 0.25f), CUI::CORNER_ALL, 10.0f);
	RenderTools()->RenderTee(CAnimState::GetIdle(), &OwnSkinInfo, 0, vec2(1, 0), vec2(Label.x + 30.0f, Label.y + 28.0f));
	Label.HSplitTop(15.0f, 0, &Label);;
	Label.VSplitLeft(70.0f, 0, &Label);
	UI()->DoLabelScaled(&Label, pSkin, 14.0f, -1, 150.0f);

	// custom colour selector
	View.HSplitTop(20.0f, 0, &View);
	View.HSplitTop(20.0f, &Button, &View);
	Button.VSplitLeft(230.0f, &Button, 0);
	if (DoButton_CheckBox(pUseCustomColor, Localize("Custom colors"), *pUseCustomColor, &Button))
	{
		*pUseCustomColor = *pUseCustomColor ? 0 : 1;
		m_NeedSendinfo = true;
	}

	View.HSplitTop(5.0f, 0, &View);
	//View.HSplitTop(82.5f, &Label, &View);
	if (*pUseCustomColor)
	{
		CUIRect aRects[2];
		Label.VSplitMid(&aRects[0], &aRects[1]);
		View.VSplitMid(&aRects[0], 0);
		aRects[0].HSplitMid(&aRects[0], &aRects[1]);

		aRects[0].VSplitRight(10.0f, &aRects[0], 0);
		aRects[1].VSplitRight(10.0f, &aRects[1], 0);

		int *paColors[2];
		paColors[0] = pColorBody;
		paColors[1] = pColorFeet;

		const char *paParts[] = {
			Localize("Body"),
			Localize("Feet") };
		const char *paLabels[] = {
			Localize("Hue"),
			Localize("Sat."),
			Localize("Lht.") };
		static int s_aColorSlider[numID][2][3] = { { 0 } };

		for (int i = 0; i < 2; i++)
		{
			aRects[i].HSplitTop(20.0f, &Label, &aRects[i]);
			UI()->DoLabelScaled(&Label, paParts[i], 14.0f, -1);
			aRects[i].VSplitLeft(20.0f, 0, &aRects[i]);
			aRects[i].HSplitTop(2.5f, 0, &aRects[i]);

			int PrevColor = *paColors[i];
			int Color = 0;
			for (int s = 0; s < 3; s++)
			{
				aRects[i].HSplitTop(20.0f, &Label, &aRects[i]);
				Label.VSplitLeft(100.0f, &Label, &Button);
				Button.HMargin(2.0f, &Button);

				float k = ((PrevColor >> ((2 - s) * 8)) & 0xff) / 255.0f;
				k = DoScrollbarH(&s_aColorSlider[Page][i][s], &Button, k);
				Color <<= 8;
				Color += clamp((int)(k * 255), 0, 255);
				UI()->DoLabelScaled(&Label, paLabels[s], 14.0f, -1);
			}

			if (PrevColor != Color)
				m_NeedSendinfo = true;

			*paColors[i] = Color;
		}
	}

	// skin selector
	MainView.VSplitMid(0, &MainView);

	// player name
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(150.0f, &Button, 0);
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Name"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0, -1);
	static float s_OffsetName = 0.0f;
	if (DoEditBox(pName, &Button, pName, sizeof(g_Config.m_PlayerName), 14.0f, &s_OffsetName))
		m_NeedSendinfo = true;

	// player clan
	MainView.HSplitTop(5.0f, 0, &MainView);
	MainView.HSplitTop(20.0f, &Button, &MainView);
	Button.VSplitLeft(80.0f, &Label, &Button);
	Button.VSplitLeft(150.0f, &Button, 0);
	str_format(aBuf, sizeof(aBuf), "%s:", Localize("Clan"));
	UI()->DoLabelScaled(&Label, aBuf, 14.0, -1);
	static float s_OffsetClan = 0.0f;
	if (DoEditBox(pClan, &Button, pClan, sizeof(g_Config.m_PlayerClan), 14.0f, &s_OffsetClan))
		m_NeedSendinfo = true;

	MainView.HSplitTop(10.0f, 0, &MainView);
	static bool s_InitSkinlist[numID] = { false };
	static sorted_array<const CSkins::CSkin *> s_paSkinList[numID];
	static float s_ScrollValue[numID] = { 0.0f };

	if (!s_InitSkinlist[Page])
	{
		s_paSkinList[Page].clear();
		for (int i = 0; i < m_pClient->m_pSkins->Num(); ++i)
		{
			const CSkins::CSkin *s = m_pClient->m_pSkins->Get(i);
			// no special skins
			if (s->m_aName[0] == 'x' && s->m_aName[1] == '_')
				continue;
			s_paSkinList[Page].add(s);
		}
		s_InitSkinlist[Page] = true;
	}

	int OldSelected = -1;
	UiDoListboxStart(&s_InitSkinlist[Page], &MainView, 50.0f, Localize("Skins"), "", s_paSkinList[Page].size(), 4, OldSelected, s_ScrollValue[Page]);

	for (int i = 0; i < s_paSkinList[Page].size(); i++)
	{

		const CSkins::CSkin *s = s_paSkinList[Page][i];

		if (s == 0)
			continue;

		if (str_comp(s->m_aName, pSkin) == 0)
			OldSelected = i;

		CListboxItem Item = UiDoListboxNextItem(&s_paSkinList[Page][i], OldSelected == i);
		if (Item.m_Visible)
		{
			CTeeRenderInfo Info;
			if (*pUseCustomColor)
			{
				Info.m_Texture = s->m_ColorTexture;
				Info.m_ColorBody = m_pClient->m_pSkins->GetColorV4(*pColorBody);
				Info.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(*pColorFeet);
			}
			else
			{
				Info.m_Texture = s->m_OrgTexture;
				Info.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				Info.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			Info.m_Size = UI()->Scale()*50.0f;
			Item.m_Rect.HSplitTop(5.0f, 0, &Item.m_Rect); // some margin from the top
			RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, 0, vec2(1.0f, 0.0f), vec2(Item.m_Rect.x + Item.m_Rect.w / 2, Item.m_Rect.y + Item.m_Rect.h / 2));

			if (g_Config.m_Debug)
			{
				vec3 BloodColor = *pUseCustomColor ? m_pClient->m_pSkins->GetColorV3(*pColorBody) : s->m_BloodColor;
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				Graphics()->SetColor(BloodColor.r, BloodColor.g, BloodColor.b, 1.0f);
				IGraphics::CQuadItem QuadItem(Item.m_Rect.x, Item.m_Rect.y, 12.0f, 12.0f);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ScrollValue[Page], 0);
	if (OldSelected != NewSelected)
	{
		str_format(pSkin, sizeof(s_paSkinList[Page][NewSelected]->m_aName), s_paSkinList[Page][NewSelected]->m_aName);
		m_NeedSendinfo = true;
	}
}

void CMenus::Render13x37Bots(CUIRect MainView)
{
	CUIRect Temp, TabBar, Button, Label;
	static int Page = 0;
	const char *aTabs[] = {
		Localize("Aimbot"),
		Localize("Spinbot"),
		Localize("Chatbot"),
		Localize("Movebot"), };
	int NumTabs = (int)(sizeof(aTabs) / sizeof(*aTabs));


	MainView.VSplitLeft(140.0f, &TabBar, &MainView);
	TabBar.VSplitRight(2.0f, &TabBar, &Button);
	RenderTools()->DrawUIRect(&Button, vec4(0.0f, 0.8f, 0.6f, 0.5f), 0, 0);


	//RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);

	for(int i = 0; i < NumTabs; i++)
	{
		TabBar.HSplitTop(24.0f, &Button, &TabBar);

		if (DoButton_MenuTab(aTabs[i], aTabs[i], Page == i, &Button, 0))
			Page = i;
	}

	MainView.Margin(10.0f, &MainView);
	
	if(Page == 0) //aimbot
	{
		MainView.HSplitTop(20.0f, &Button, &MainView);
		Button.VSplitLeft(120.0f, &Button, &Temp);
		if (DoButton_CheckBox(&g_Config.m_XAimbot, Localize("Aimbot"), g_Config.m_XAimbot, &Button))
			g_Config.m_XAimbot ^= 1;
		if(DoButton_CheckBox(&g_Config.m_XTriggerbot, Localize("Triggerbot"), g_Config.m_XTriggerbot, &Temp))
			g_Config.m_XTriggerbot ^= 1;
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XAimbotHotkey, Localize("Hotkey"), g_Config.m_XAimbotHotkey, &Button))
			g_Config.m_XAimbotHotkey ^= 1;
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XAimbotWallcheck, Localize("Wallcheck"), g_Config.m_XAimbotWallcheck, &Button))
			g_Config.m_XAimbotWallcheck ^= 1;
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XAimbotTeamcheck, Localize("Teamcheck"), g_Config.m_XAimbotTeamcheck, &Button))
			g_Config.m_XAimbotTeamcheck ^= 1;
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if(DoButton_CheckBox(&g_Config.m_XAimbotEnemycheck, Localize("Aim on marked enemies only"), g_Config.m_XAimbotEnemycheck, &Button))
			g_Config.m_XAimbotEnemycheck ^= 1;
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if(DoButton_CheckBox(&g_Config.m_XAimbotFriendcheck, Localize("Do not aim on friends"), g_Config.m_XAimbotFriendcheck, &Button))
			g_Config.m_XAimbotFriendcheck ^= 1;
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if(DoButton_CheckBox(&g_Config.m_XAimbotFNG, Localize("FNG mode (do not aim on frozen targets"), g_Config.m_XAimbotFNG, &Button))
			g_Config.m_XAimbotFNG ^= 1;

		MainView.HSplitTop(20.0f, &Button, &MainView);
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if(DoButton_CheckBox(&g_Config.m_XAimbotRangeFromTuning, Localize("Set aimrange to laser reach from tuning"), g_Config.m_XAimbotRangeFromTuning, &Button))
			g_Config.m_XAimbotRangeFromTuning ^= 1;

		MainView.HSplitTop(20.0f, &Button, &MainView);
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "%s: %d", Localize("Aimrange"), g_Config.m_XAimbotRange);
		if (g_Config.m_XAimbotRange == 0)
			str_format(aBuf, sizeof(aBuf), "%s: Unlimited", Localize("Aimrange"));
		UI()->DoLabelScaled(&Button, aBuf, 13.0f, -1);
		Button.VSplitRight(15.0f, &Button, 0);
		Button.VSplitLeft(140.0f, &Label, &Button);
		Button.HMargin(2.0f, &Button);
		g_Config.m_XAimbotRange = (int)(DoScrollbarH(&g_Config.m_XAimbotRange, &Button, g_Config.m_XAimbotRange / 2500.0f)*2500.0f);
	}
	else if (Page == 1)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Direction (current = %s)", g_Config.m_XSpinbotDirection ? "Left" : "Right");
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XSpinbot, Localize("Spinbot"), g_Config.m_XSpinbot, &Button))
			g_Config.m_XSpinbot ^= 1;
		MainView.HSplitTop(20.0f, &Button, &MainView);
		Button.VSplitLeft(120.0f, &Button, &Temp);
		if (DoButton_CheckBox(&g_Config.m_XSpinbotDirection, "Left", g_Config.m_XSpinbotDirection == 0, &Button))
			g_Config.m_XSpinbotDirection = 0;
		static int s_ButtonRight = 0;
		if(DoButton_CheckBox(&s_ButtonRight, "Right", g_Config.m_XSpinbotDirection == 1, &Temp))
			g_Config.m_XSpinbotDirection = 1;
	}
	else if (Page == 2)
	{
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XChatbotRepeat, Localize("Repeat chat"), g_Config.m_XChatbotRepeat, &Button))
			g_Config.m_XChatbotRepeat ^= 1;
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XChatbotFlame, Localize("Flame on death"), g_Config.m_XChatbotFlame, &Button))
		{
			g_Config.m_XChatbotFlame ^= 1;
			g_Config.m_XChatbotPraise = 0;
		}
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XChatbotPraise, Localize("Praise on death"), g_Config.m_XChatbotPraise, &Button))
		{
			g_Config.m_XChatbotPraise ^= 1;
				g_Config.m_XChatbotFlame = 0;
		}
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XChatbotPingAll, Localize("Ping all"), g_Config.m_XChatbotPingAll, &Button))
			g_Config.m_XChatbotPingAll ^= 1;

	}
	else if (Page == 3)
	{
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XMovebotFly, Localize("Flybot"), g_Config.m_XMovebotFly, &Button))
		{
			g_Config.m_XMovebotFly ^= 1;
			g_Config.m_XMovebotBalance = 0;

		}
		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XMovebotBalance, Localize("Balancebot"), g_Config.m_XMovebotBalance, &Button))
		{
			g_Config.m_XMovebotBalance ^= 1;
			g_Config.m_XMovebotFly = 0;
		}

		MainView.HSplitTop(20.0f, &Button, &MainView);
		if (DoButton_CheckBox(&g_Config.m_XMovebotRocketjump, Localize("Rocketjump"), g_Config.m_XMovebotRocketjump, &Button))
			g_Config.m_XMovebotRocketjump ^= 1;
	}
}

void CMenus::Render13x37(CUIRect MainView)
{
	CUIRect Temp, TabBar, Button;
	const char *aTabs[] = {
		Localize("Extras"),
		Localize("ESP"),
		Localize("Tee & HUD"),
		Localize("Bots"),
		Localize("Dummy"),
		Localize("Identity"),
		Localize("Econ"),
		Localize("Info"),};
	int NumTabs = (int)(sizeof(aTabs)/sizeof(*aTabs));

	RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);
	
	MainView.HSplitTop(26.0f, &TabBar, &MainView);
	for(int i = 0; i < NumTabs; i++)
	{
		TabBar.VSplitLeft(MainView.w/NumTabs, &Button, &TabBar);
		
		if(DoButton_MenuTab(aTabs[i], aTabs[i], m_XPage == i, &Button, i ? i == NumTabs-1 ? CUI::CORNER_TR : 0 : CUI::CORNER_TL))
			m_XPage = i;
	}

	MainView.HSplitTop(2.0f, &Button, &MainView);
	RenderTools()->DrawUIRect(&Button, vec4(0.0f, 0.8f, 0.6f, 0.5f), 0, 0);

	if(m_XPage == 0)
		Render13x37Extras(MainView);
	else if(m_XPage == 1)
		Render13x37Esp(MainView);
	else if(m_XPage == 2)
		 Render13x37Tee(MainView);
	else if(m_XPage == 3)
		Render13x37Bots(MainView);
	else if (m_XPage == 4)
		Render13x37Dummy(MainView);
	else if(m_XPage == 5)
		Render13x37Identity(MainView);
	else if(m_XPage == 6)
		Render13x37Econ(MainView);
}
