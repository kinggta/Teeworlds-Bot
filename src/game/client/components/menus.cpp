/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <engine/config.h>
#include <engine/editor.h>
#include <engine/engine.h>
#include <engine/friends.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <game/version.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/generated/client_data.h>
#include <game/client/components/camera.h>
#include <game/client/components/sounds.h>
#include <game/client/components/maplayers.h>
#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/lineinput.h>
#include <game/localization.h>
#include <mastersrv/mastersrv.h>

#include "countryflags.h"
#include "menus.h"
#include "skins.h"

vec4 CMenus::ms_GuiColor;
vec4 CMenus::ms_ColorTabbarInactiveOutgame;
vec4 CMenus::ms_ColorTabbarActiveOutgame;
vec4 CMenus::ms_ColorTabbarInactive;
vec4 CMenus::ms_ColorTabbarActive = vec4(0,0,0,0.5f);
vec4 CMenus::ms_ColorTabbarInactiveIngame;
vec4 CMenus::ms_ColorTabbarActiveIngame;

float CMenus::ms_ButtonHeight = 25.0f;
float CMenus::ms_ListheaderHeight = 17.0f;
float CMenus::ms_FontmodHeight = 0.8f;

IInput::CEvent CMenus::m_aInputEvents[MAX_INPUTEVENTS];
int CMenus::m_NumInputEvents;

int64 CMenus::ms_LastLogin = 0.0f;

static int gs_TextureBlob = -1;
static int gs_TextureAvaDefault = -1;
static int gs_TextureAvaOffline = -1;


CMenus::CMenus()
{
	m_Popup = POPUP_NONE;
	m_ActivePage = PAGE_NONE;
	m_GamePage = PAGE_SERVER;
	m_BrowserPage = BROWSER_INTERNET;

	m_NeedRestartGraphics = false;
	m_NeedRestartSound = false;
	m_NeedSendinfo = false;
	m_MenuActive = true;
	m_UseMouseButtons = true;

	m_EscapePressed = false;
	m_EnterPressed = false;
	m_DeletePressed = false;
	m_NumInputEvents = 0;

	m_UseShellmap = true;

	m_LastInput = time_get();

	str_copy(m_aCurrentDemoFolder, "demos", sizeof(m_aCurrentDemoFolder));
	m_aCallvoteReason[0] = 0;

	m_FriendlistSelectedIndex = -1;
}

vec4 CMenus::ButtonColorMul(const void *pID)
{
	if(UI()->ActiveItem() == pID)
		return vec4(1,1,1,0.5f);
	else if(UI()->HotItem() == pID)
		return vec4(1,1,1,1.5f);
	return vec4(1,1,1,1);
}

int CMenus::DoButton_Icon(int ImageId, int SpriteId, const CUIRect *pRect)
{
	Graphics()->TextureSet(g_pData->m_aImages[ImageId].m_Id);

	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SpriteId);
	IGraphics::CQuadItem QuadItem(pRect->x, pRect->y, pRect->w, pRect->h);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

	return 0;
}

int CMenus::DoButton_Toggle(const void *pID, int Checked, const CUIRect *pRect, bool Active, const char *pTip)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GUIBUTTONS].m_Id);
	Graphics()->QuadsBegin();
	if(!Active)
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.5f);
	RenderTools()->SelectSprite(Checked?SPRITE_GUIBUTTON_ON:SPRITE_GUIBUTTON_OFF);
	IGraphics::CQuadItem QuadItem(pRect->x, pRect->y, pRect->w, pRect->h);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	if(UI()->HotItem() == pID && Active)
	{
		RenderTools()->SelectSprite(SPRITE_GUIBUTTON_HOVER);
		IGraphics::CQuadItem QuadItem(pRect->x, pRect->y, pRect->w, pRect->h);
		Graphics()->QuadsDrawTL(&QuadItem, 1);
	}
	Graphics()->QuadsEnd();

	return Active ? UI()->DoButtonLogic(pID, "", Checked, pRect) : 0;
}

int CMenus::DoButton_Menu(const void *pID, const char *pText, int Checked, const CUIRect *pRect, const char *pTip)
{
	RenderTools()->DrawUIRect(pRect, vec4(1,1,1,0.5f)*ButtonColorMul(pID), CUI::CORNER_ALL, 5.0f);
	CUIRect Temp;
	pRect->HMargin(pRect->h>=20.0f?2.0f:1.0f, &Temp);
	UI()->DoLabel(&Temp, pText, Temp.h*ms_FontmodHeight, 0);
	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

void CMenus::DoButton_KeySelect(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
	RenderTools()->DrawUIRect(pRect, vec4(1,1,1,0.5f)*ButtonColorMul(pID), CUI::CORNER_ALL, 5.0f);
	CUIRect Temp;
	pRect->HMargin(1.0f, &Temp);
	UI()->DoLabel(&Temp, pText, Temp.h*ms_FontmodHeight, 0);
}

int CMenus::DoButton_MenuTab(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Corners, const char *pTip)
{
	if(Checked)
		RenderTools()->DrawUIRect(pRect, vec4(0.0f, 0.8f, 0.6f, 0.5f), Corners, 10.0f);
	else
		RenderTools()->DrawUIRect(pRect, ms_ColorTabbarInactive, Corners, 10.0f);
	CUIRect Temp;
	pRect->HMargin(2.0f, &Temp);
	UI()->DoLabel(&Temp, pText, Temp.h*ms_FontmodHeight, 0);

	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

int CMenus::DoButton_MenuMain(const void *pID, const char *pText, int Checked, const CUIRect *pRect, vec3 Color, int Page, int Align, int SpriteID, const char *pTip)
{
	CUIRect Button = *pRect, Label;
	float var[3];
	int CurPage = -1;
	if(Client()->State() == IClient::STATE_OFFLINE)
		CurPage = g_Config.m_UiPage;
	else
		CurPage = m_GamePage;

	//RenderTools()->DrawUIRect(&Button, vec4(0.0f, 0.0f, 0.0f, 0.5f), CUI::CORNER_ALL, 5.0f);
	Button.HSplitBottom(24.0f, &Button, &Label);
	RenderTools()->DrawUIRect(&Label, vec4(0.0f, 0.0f, 0.0f, 0.5f), CUI::CORNER_B, 5.0f);
	if (Page == CurPage || UI()->HotItem() == pID)
	{
		Label.h +=  2.0f;
		Label.y -= 1.0f;		
	}
	Label.Margin(2.0f, &Label);
	UI()->DoLabel(&Label, pText, Label.h*ms_FontmodHeight, Align);

	

	Graphics()->ShaderSet(IGraphics::SHADER_COLOR);
	Graphics()->ShaderBegin();
	
	var[0] = 1.0f;
	Graphics()->ShaderUniformSet("u_IsTex", var, 1);
	var[0] = 0.5f;
	var[1] = 0.5f;
	var[2] = 0.5f;
	Graphics()->ShaderUniformSet("u_Color", var, 3);

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_X_MENU_BUTTONS].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SpriteID);
	Graphics()->QuadsDraw(&IGraphics::CQuadItem(Button.x+Button.w/2, Button.y+Button.h/2, Button.w, Button.h), 1);

	if(CurPage == Page || UI()->HotItem() == pID)
	{
		if(CurPage == Page)
		{
			var[0] = Color.x;
			var[1] = Color.y;
			var[2] = Color.z;
			Graphics()->ShaderUniformSet("u_Color", var, 3);
			SpriteID++;
		}
		else if(UI()->HotItem() == pID)
		{
			var[0] = Color.x;
			var[1] = Color.y;
			var[2] = Color.z;
			Graphics()->ShaderUniformSet("u_Color", var, 3);
		}
			
		RenderTools()->SelectSprite(SpriteID);
		Graphics()->QuadsDraw(&IGraphics::CQuadItem(Button.x+Button.w/2, Button.y+Button.h/2, Button.w, Button.h), 1);
	}

	//(CurPage==Page?2:(UI()->HotItem()==pID?1:0)))
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();

	//RenderTools()->DrawUIRect(pRect, vec4(1.0f,1.0f,1.0f,1.0f), 0, -1);
	//RenderTools()->DrawUIRect(pRect, vec4(1.0f,1.0f,1.0f,0.15f)*ButtonColorMul(pID), 0, 3.0f);

	
	if(pID == UI()->HotItem() || pID == UI()->ActiveItem())
		str_format(m_aToolTip, sizeof(m_aToolTip), pTip);
	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

int CMenus::DoButton_GridHeader(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
//void CMenus::ui_draw_grid_header(const void *id, const char *text, int checked, const CUIRect *r, const void *extra)
{
	if(Checked)
		RenderTools()->DrawUIRect(pRect, vec4(1,1,1,0.5f), CUI::CORNER_T, 5.0f);
	CUIRect t;
	pRect->VSplitLeft(5.0f, 0, &t);
	UI()->DoLabel(&t, pText, pRect->h*ms_FontmodHeight, -1);
	return UI()->DoButtonLogic(pID, pText, Checked, pRect);
}

int CMenus::DoButton_CheckBox_Common(const void *pID, const char *pText, const char *pBoxText, const CUIRect *pRect, const char *pTip)
//void CMenus::ui_draw_checkbox_common(const void *id, const char *text, const char *boxtext, const CUIRect *r, const void *extra)
{
	CUIRect c = *pRect;
	CUIRect t = *pRect;
	c.w = c.h;
	t.x += c.w;
	t.w -= c.w;
	t.VSplitLeft(5.0f, 0, &t);

	c.Margin(2.0f, &c);
	RenderTools()->DrawUIRect(&c, vec4(1,1,1,0.25f)*ButtonColorMul(pID), CUI::CORNER_ALL, 3.0f);
	c.y += 2;
	UI()->DoLabel(&c, pBoxText, pRect->h*ms_FontmodHeight*0.6f, 0);
	UI()->DoLabel(&t, pText, pRect->h*ms_FontmodHeight*0.8f, -1);
	return UI()->DoButtonLogic(pID, pText, 0, pRect);
}

int CMenus::DoButton_CheckBox(const void *pID, const char *pText, int Checked, const CUIRect *pRect, const char *pTip)
{
	return DoButton_CheckBox_Common(pID, pText, Checked?"X":"", pRect, pTip);
}


int CMenus::DoButton_CheckBox_Number(const void *pID, const char *pText, int Checked, const CUIRect *pRect, const char *pTip)
{
	char aBuf[16];
	str_format(aBuf, sizeof(aBuf), "%d", Checked);
	return DoButton_CheckBox_Common(pID, pText, aBuf, pRect);
}

int CMenus::DoEditBox(void *pID, const CUIRect *pRect, char *pStr, unsigned StrSize, float FontSize, float *Offset, bool Hidden, int Corners)
{
	int Inside = UI()->MouseInside(pRect);
	bool ReturnValue = false;
	bool UpdateOffset = false;
	static int s_AtIndex = 0;
	static bool s_DoScroll = false;
	static float s_ScrollStart = 0.0f;

	FontSize *= UI()->Scale();

	if(UI()->LastActiveItem() == pID)
	{
		int Len = str_length(pStr);
		if(Len == 0)
			s_AtIndex = 0;

		if(Inside && UI()->MouseButton(0))
		{
			s_DoScroll = true;
			s_ScrollStart = UI()->MouseX();
			int MxRel = (int)(UI()->MouseX() - pRect->x);

			for(int i = 1; i <= Len; i++)
			{
				if(TextRender()->TextWidth(0, FontSize, pStr, i) - *Offset > MxRel)
				{
					s_AtIndex = i - 1;
					break;
				}

				if(i == Len)
					s_AtIndex = Len;
			}
		}
		else if(!UI()->MouseButton(0))
			s_DoScroll = false;
		else if(s_DoScroll)
		{
			// do scrolling
			if(UI()->MouseX() < pRect->x && s_ScrollStart-UI()->MouseX() > 10.0f)
			{
				s_AtIndex = max(0, s_AtIndex-1);
				s_ScrollStart = UI()->MouseX();
				UpdateOffset = true;
			}
			else if(UI()->MouseX() > pRect->x+pRect->w && UI()->MouseX()-s_ScrollStart > 10.0f)
			{
				s_AtIndex = min(Len, s_AtIndex+1);
				s_ScrollStart = UI()->MouseX();
				UpdateOffset = true;
			}
		}

		for(int i = 0; i < m_NumInputEvents; i++)
		{
			Len = str_length(pStr);
			int NumChars = Len;
			ReturnValue |= CLineInput::Manipulate(m_aInputEvents[i], pStr, StrSize, StrSize, &Len, &s_AtIndex, &NumChars);
		}
	}

	bool JustGotActive = false;

	if(UI()->ActiveItem() == pID)
	{
		if(!UI()->MouseButton(0))
		{
			s_AtIndex = min(s_AtIndex, str_length(pStr));
			s_DoScroll = false;
			UI()->SetActiveItem(0);
		}
	}
	else if(UI()->HotItem() == pID)
	{
		if(UI()->MouseButton(0))
		{
			if (UI()->LastActiveItem() != pID)
				JustGotActive = true;
			UI()->SetActiveItem(pID);
		}
	}

	if(Inside)
		UI()->SetHotItem(pID);

	CUIRect Textbox = *pRect;
	RenderTools()->DrawUIRect(&Textbox, vec4(1, 1, 1, 0.5f), Corners, 3.0f);
	Textbox.VMargin(2.0f, &Textbox);
	Textbox.HMargin(2.0f, &Textbox);

	const char *pDisplayStr = pStr;
	char aStars[128];

	if(Hidden)
	{
		unsigned s = str_length(pStr);
		if(s >= sizeof(aStars))
			s = sizeof(aStars)-1;
		for(unsigned int i = 0; i < s; ++i)
			aStars[i] = '*';
		aStars[s] = 0;
		pDisplayStr = aStars;
	}

	// check if the text has to be moved
	if(UI()->LastActiveItem() == pID && !JustGotActive && (UpdateOffset || m_NumInputEvents))
	{
		float w = TextRender()->TextWidth(0, FontSize, pDisplayStr, s_AtIndex);
		if(w-*Offset > Textbox.w)
		{
			// move to the left
			float wt = TextRender()->TextWidth(0, FontSize, pDisplayStr, -1);
			do
			{
				*Offset += min(wt-*Offset-Textbox.w, Textbox.w/3);
			}
			while(w-*Offset > Textbox.w);
		}
		else if(w-*Offset < 0.0f)
		{
			// move to the right
			do
			{
				*Offset = max(0.0f, *Offset-Textbox.w/3);
			}
			while(w-*Offset < 0.0f);
		}
	}
	UI()->ClipEnable(pRect);
	Textbox.x -= *Offset;

	UI()->DoLabel(&Textbox, pDisplayStr, FontSize, -1);

	// render the cursor
	if(UI()->LastActiveItem() == pID && !JustGotActive)
	{
		float w = TextRender()->TextWidth(0, FontSize, pDisplayStr, s_AtIndex);
		Textbox = *pRect;
		Textbox.VSplitLeft(2.0f, 0, &Textbox);
		Textbox.x += (w-*Offset-TextRender()->TextWidth(0, FontSize, "|", -1)/2);

		if((2*time_get()/time_freq()) % 2)	// make it blink
			UI()->DoLabel(&Textbox, "|", FontSize, -1);
	}
	UI()->ClipDisable();

	return ReturnValue;
}

float CMenus::DoScrollbarV(const void *pID, const CUIRect *pRect, float Current)
{
	CUIRect Handle;
	static float OffsetY;
	pRect->HSplitTop(33, &Handle, 0);

	Handle.y += (pRect->h-Handle.h)*Current;

	// logic
	float ReturnValue = Current;
	int Inside = UI()->MouseInside(&Handle);

	if(UI()->ActiveItem() == pID)
	{
		if(!UI()->MouseButton(0))
			UI()->SetActiveItem(0);

		float Min = pRect->y;
		float Max = pRect->h-Handle.h;
		float Cur = UI()->MouseY()-OffsetY;
		ReturnValue = (Cur-Min)/Max;
		if(ReturnValue < 0.0f) ReturnValue = 0.0f;
		if(ReturnValue > 1.0f) ReturnValue = 1.0f;
	}
	else if(UI()->HotItem() == pID)
	{
		if(UI()->MouseButton(0))
		{
			UI()->SetActiveItem(pID);
			OffsetY = UI()->MouseY()-Handle.y;
		}
	}

	if(Inside)
		UI()->SetHotItem(pID);

	// render
	CUIRect Rail;
	pRect->VMargin(5.0f, &Rail);
	RenderTools()->DrawUIRect(&Rail, vec4(1,1,1,0.25f), 0, 0.0f);

	CUIRect Slider = Handle;
	Slider.w = Rail.x-Slider.x;
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f), CUI::CORNER_L, 2.5f);
	Slider.x = Rail.x+Rail.w;
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f), CUI::CORNER_R, 2.5f);

	Slider = Handle;
	Slider.Margin(5.0f, &Slider);
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f)*ButtonColorMul(pID), CUI::CORNER_ALL, 2.5f);

	return ReturnValue;
}



float CMenus::DoScrollbarH(const void *pID, const CUIRect *pRect, float Current)
{
	CUIRect Handle;
	static float OffsetX;
	pRect->VSplitLeft(33, &Handle, 0);

	Handle.x += (pRect->w-Handle.w)*Current;

	// logic
	float ReturnValue = Current;
	int Inside = UI()->MouseInside(&Handle);

	if(UI()->ActiveItem() == pID)
	{
		if(!UI()->MouseButton(0))
			UI()->SetActiveItem(0);

		float Min = pRect->x;
		float Max = pRect->w-Handle.w;
		float Cur = UI()->MouseX()-OffsetX;
		ReturnValue = (Cur-Min)/Max;
		if(ReturnValue < 0.0f) ReturnValue = 0.0f;
		if(ReturnValue > 1.0f) ReturnValue = 1.0f;
	}
	else if(UI()->HotItem() == pID)
	{
		if(UI()->MouseButton(0))
		{
			UI()->SetActiveItem(pID);
			OffsetX = UI()->MouseX()-Handle.x;
		}
	}

	if(Inside)
		UI()->SetHotItem(pID);

	// render
	CUIRect Rail;
	pRect->HMargin(5.0f, &Rail);
	RenderTools()->DrawUIRect(&Rail, vec4(1,1,1,0.25f), 0, 0.0f);

	CUIRect Slider = Handle;
	Slider.h = Rail.y-Slider.y;
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f), CUI::CORNER_T, 2.5f);
	Slider.y = Rail.y+Rail.h;
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f), CUI::CORNER_B, 2.5f);

	Slider = Handle;
	Slider.Margin(5.0f, &Slider);
	RenderTools()->DrawUIRect(&Slider, vec4(1,1,1,0.25f)*ButtonColorMul(pID), CUI::CORNER_ALL, 2.5f);

	return ReturnValue;
}

int CMenus::DoKeyReader(void *pID, const CUIRect *pRect, int Key)
{
	// process
	static void *pGrabbedID = 0;
	static bool MouseReleased = true;
	static int ButtonUsed = 0;
	int Inside = UI()->MouseInside(pRect);
	int NewKey = Key;

	if(!UI()->MouseButton(0) && !UI()->MouseButton(1) && pGrabbedID == pID)
		MouseReleased = true;

	if(UI()->ActiveItem() == pID)
	{
		if(m_Binder.m_GotKey)
		{
			// abort with escape key
			if(m_Binder.m_Key.m_Key != KEY_ESCAPE)
				NewKey = m_Binder.m_Key.m_Key;
			m_Binder.m_GotKey = false;
			UI()->SetActiveItem(0);
			MouseReleased = false;
			pGrabbedID = pID;
		}

		if(ButtonUsed == 1 && !UI()->MouseButton(1))
		{
			if(Inside)
				NewKey = 0;
			UI()->SetActiveItem(0);
		}
	}
	else if(UI()->HotItem() == pID)
	{
		if(MouseReleased)
		{
			if(UI()->MouseButton(0))
			{
				m_Binder.m_TakeKey = true;
				m_Binder.m_GotKey = false;
				UI()->SetActiveItem(pID);
				ButtonUsed = 0;
			}

			if(UI()->MouseButton(1))
			{
				UI()->SetActiveItem(pID);
				ButtonUsed = 1;
			}
		}
	}

	if(Inside)
		UI()->SetHotItem(pID);

	// draw
	if (UI()->ActiveItem() == pID && ButtonUsed == 0)
		DoButton_KeySelect(pID, "???", 0, pRect);
	else
	{
		if(Key == 0)
			DoButton_KeySelect(pID, "", 0, pRect);
		else
			DoButton_KeySelect(pID, Input()->KeyName(Key), 0, pRect);
	}
	return NewKey;
}


int CMenus::RenderMenubar(CUIRect r)
{
	CUIRect Box = r;
	CUIRect Button;

	static int NewPage = -1;//PAGE_NONE;//has to be -1
	float Width = 100.0f;
	float Padding = (Box.w - 7*Width) * 0.5f;

	m_ActivePage = g_Config.m_UiPage;
	
	if(Client()->State() != IClient::STATE_OFFLINE)
		m_ActivePage = m_GamePage;

	/*if(Client()->State() == IClient::STATE_OFFLINE)
	{*/
		// offline menus
		Box.VSplitLeft(Padding, &Button, &Box);
		Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_NewsButton=0;
		if (Client()->State() == IClient::STATE_OFFLINE)
		{
			if (DoButton_MenuMain(&s_NewsButton, Localize("News"), m_ActivePage == PAGE_NEWS, &Button, vec3(0.2f, 0.2f, 1.0f), PAGE_NEWS, 0, SPRITE_X_MAINBUTTON_NEWS_HOT,
				Localize("Get news on Teeworlds and 13x37")))
				NewPage == PAGE_NEWS ? NewPage = PAGE_NONE : NewPage = PAGE_NEWS;
		}
		else
		{
			if (DoButton_MenuMain(&s_NewsButton, Localize("Server"), m_ActivePage == PAGE_SERVER, &Button, vec3(0.1f, 0.1f, 0.75f), PAGE_SERVER, 0, SPRITE_X_MAINBUTTON_NEWS_HOT,
				Localize("Get info on the current server and players")))
				NewPage == PAGE_SERVER ? NewPage = PAGE_NONE : NewPage = PAGE_SERVER;
		}

		Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_BrowserButton=0;
		if(DoButton_MenuMain(&s_BrowserButton, Localize("Play"), m_ActivePage==PAGE_BROWSER, &Button, vec3(0.8f, 0.8f, 0.0f), PAGE_BROWSER, 0, SPRITE_X_MAINBUTTON_PLAY_HOT,
			Localize("Play together with other Tees from all around the world and show your skills")))
		{
			ServerBrowser()->Refresh(m_BrowserPage);
			NewPage == PAGE_BROWSER ? NewPage = PAGE_NONE : NewPage = PAGE_BROWSER;
		}

		Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_EditorButton=0;
		if(DoButton_MenuMain(&s_EditorButton, Localize("Editor"), 0, &Button,  vec3(0.0f, 0.8f, 0.0f), -1, 0, SPRITE_X_MAINBUTTON_EDITOR_HOT,
			Localize("Build, play, and share your own maps")))
		{
			g_Config.m_ClEditor = 1;
			Input()->MouseModeRelative();
		}
		
		Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_DemosButton=0;
		if(DoButton_MenuMain(&s_DemosButton, Localize("Demos"), m_ActivePage==PAGE_DEMOS, &Button, vec3(0.8f, 0.0f, 0.0f), PAGE_DEMOS, 0, SPRITE_X_MAINBUTTON_DEMO_HOT,
			Localize("Watch your recorded matches")))
		{
			DemolistPopulate();
			NewPage == PAGE_DEMOS ? NewPage = PAGE_NONE : NewPage = PAGE_DEMOS;
		}

		Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_13x37Button = 0;
		if (DoButton_MenuMain(&s_13x37Button, Localize("xClient"), m_ActivePage == PAGE_13X37, &Button, vec3(0.0f, 1.0f, 0.5f), PAGE_13X37, 0, SPRITE_X_MAINBUTTON_13X37_HOT,
			Localize("Special options of 13x37")))
			NewPage == PAGE_13X37 ? NewPage = PAGE_NONE : NewPage = PAGE_13X37;

		Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_SettingsButton = 0;
		if (DoButton_MenuMain(&s_SettingsButton, Localize("Settings"), m_ActivePage == PAGE_SETTINGS, &Button, vec3(0.8f, 0.8f, 0.8f), PAGE_SETTINGS, 0, SPRITE_X_MAINBUTTON_SETTINGS_HOT,
			Localize("Set your language, graphic and sound options")))
			NewPage == PAGE_SETTINGS ? NewPage = PAGE_NONE : NewPage = PAGE_SETTINGS;

		Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_QuitLabel = rand() % 3;
		static int s_QuitButton = 0;
		if (UI()->HotItem() != &s_QuitButton)
			s_QuitLabel = rand() % 3;
		if (DoButton_MenuMain(&s_QuitButton, Localize("Quit"), 0, &Button, vec3(0.7f, 0.2f, 0.2f), POPUP_QUIT, 0, SPRITE_X_MAINBUTTON_QUIT_HOT, m_aQuitLabels[s_QuitLabel]))
			m_Popup = POPUP_QUIT;
	//}
	//else
	//{
		// online menus
		Width = UI()->Screen()->w / 9; //num buttons + 2

		/*Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_GameButton=0;
		if (DoButton_MenuMain(&s_GameButton, Localize("Game"), m_ActivePage == PAGE_GAME, &Button, vec3(0.0f, 1.0f, 0.5f), PAGE_GAME, 0, -1,
			0))
			NewPage == PAGE_GAME ? NewPage = PAGE_NONE : NewPage = PAGE_GAME;

		Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_PlayersButton=0;
		if (DoButton_MenuMain(&s_PlayersButton, Localize("Players"), m_ActivePage == PAGE_PLAYERS, &Button, vec3(0.0f, 1.0f, 0.5f), PAGE_PLAYERS, 0, -1,
			0))
			NewPage == PAGE_PLAYERS ? NewPage = PAGE_NONE : NewPage = PAGE_PLAYERS;

		Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_ServerInfoButton=0;
		if (DoButton_MenuMain(&s_ServerInfoButton, Localize("Server info"), m_ActivePage == PAGE_SERVER_INFO, &Button, vec3(0.0f, 1.0f, 0.5f), PAGE_SERVER_INFO, 0, -1,
			0))
			NewPage == PAGE_SERVER_INFO ? NewPage = PAGE_NONE : NewPage = PAGE_SERVER_INFO;

		Box.VSplitLeft(Width, &Button, &Box);
		Button.Margin(4.0f, &Button);
		static int s_CallVoteButton=0;
		if (DoButton_MenuMain(&s_CallVoteButton, Localize("Call vote"), m_ActivePage == PAGE_CALLVOTE, &Button, vec3(0.0f, 1.0f, 0.5f), PAGE_13X37, 0, -1,
			0))
			NewPage == PAGE_CALLVOTE ? NewPage = PAGE_NONE : NewPage = PAGE_CALLVOTE;*/


	//}

	/*
	box.VSplitRight(110.0f, &box, &button);
	static int system_button=0;
	if (UI()->DoButton(&system_button, "System", g_Config.m_UiPage==PAGE_SYSTEM, &button))
		g_Config.m_UiPage = PAGE_SYSTEM;

	box.VSplitRight(30.0f, &box, 0);
	*/

	


	if(NewPage != -1)
	{
		if(Client()->State() == IClient::STATE_OFFLINE)
			g_Config.m_UiPage = NewPage;
		else
			m_GamePage = NewPage;
	}

	return 0;
}

void CMenus::RenderLoading()
{
	// TODO: not supported right now due to separate render thread

	static int64 LastLoadRender = 0;
	float Percent = m_LoadCurrent++/(float)m_LoadTotal;

	// make sure that we don't render for each little thing we load
	// because that will slow down loading if we have vsync
	if(time_get()-LastLoadRender < time_freq()/60)
		return;

	if(gs_TextureAvaDefault == -1)
		gs_TextureAvaDefault = Graphics()->LoadTexture("xclient/avatars/default.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);

	if(gs_TextureAvaOffline == -1)
		gs_TextureAvaOffline = Graphics()->LoadTexture("xclient/avatars/offline.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);


	LastLoadRender = time_get();

	// need up date this here to get correct
	vec3 Rgb = HslToRgb(vec3(g_Config.m_UiColorHue/255.0f, g_Config.m_UiColorSat/255.0f, g_Config.m_UiColorLht/255.0f));
	ms_GuiColor = vec4(Rgb.r, Rgb.g, Rgb.b, g_Config.m_UiColorAlpha/255.0f);

	CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	RenderBackground();

	float w = 700;
	float h = 200;
	float x = Screen.w/2-w/2;
	float y = Screen.h/2-h/2;

	Graphics()->BlendNormal();

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.50f);
	RenderTools()->DrawRoundRect(x, y, w, h, 40.0f);
	Graphics()->QuadsEnd();


	const char *pCaption = Localize("Loading");

	CUIRect r;
	r.x = x;
	r.y = y+20;
	r.w = w;
	r.h = h;
	UI()->DoLabel(&r, pCaption, 48.0f, 0, -1);

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,0.75f);
	RenderTools()->DrawRoundRect(x+40, y+h-75, (w-80)*Percent, 25, 5.0f);
	Graphics()->QuadsEnd();

	Graphics()->Swap();
}

void CMenus::RenderServer(CUIRect MainView)
{
	CUIRect Button, Box;
	MainView.VSplitLeft(10.0f, 0, &MainView);
	MainView.VSplitLeft(90.0f, &Button, &Box);
	static int s_GameButton = 0;
	if (DoButton_MenuTab(&s_GameButton, Localize("Game"), m_ServerPage == SERVER_GAME, &Button, CUI::CORNER_TL))
		m_ServerPage = SERVER_GAME;

	Box.VSplitLeft(90.0f, &Button, &Box);
	static int s_PlayersButton = 0;
	if (DoButton_MenuTab(&s_PlayersButton, Localize("Players"), m_ServerPage == SERVER_PLAYERS, &Button, 0))
		m_ServerPage = SERVER_PLAYERS;

	Box.VSplitLeft(100.0f, &Button, &Box);
	static int s_ServerInfoButton = 0;
	if (DoButton_MenuTab(&s_ServerInfoButton, Localize("Server info"), m_ServerPage == SERVER_SERVER_INFO, &Button, 0))
		m_ServerPage = SERVER_SERVER_INFO;

	Box.VSplitLeft(100.0f, &Button, &Box);
	static int s_CallVoteButton = 0;
	if (DoButton_MenuTab(&s_CallVoteButton, Localize("Call vote"), m_ServerPage == SERVER_CALLVOTE, &Button, CUI::CORNER_TR))
		m_ServerPage = SERVER_CALLVOTE;
}

void CMenus::RenderNews(CUIRect MainView)
{
	CUIRect TabBar, Button, Temp;
	static int s_Page = 1;
	static int s_PageButton1 = 0, s_PageButton2 = 0, s_PageButton3 = 0, s_PageButton4 = 0;

	
	RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_ALL, 10.0f);
	
	MainView.Margin(10.0f, &MainView);
	MainView.HSplitBottom(18.0f, &MainView, &TabBar);

	TabBar.VSplitMid(&Temp, 0);

	// Left side
	Temp.VSplitRight(4.0f, &Temp, 0);
	Temp.VSplitRight(14.0f, &Temp, &Button);
	if (DoButton_CheckBox(&s_PageButton2, Localize(""), s_Page == 2, &Button))
		s_Page = 2;

	Temp.VSplitRight(8.0f, &Temp, 0);
	Temp.VSplitRight(14.0f, &Temp, &Button);
	if (DoButton_CheckBox(&s_PageButton1, Localize(""), s_Page == 1, &Button))
		s_Page = 1;

	//Right side
	TabBar.VSplitMid(0, &Temp);
	Temp.VSplitLeft(4.0f, 0, &Temp);
	Temp.VSplitLeft(14.0f, &Button, &Temp);
	if (DoButton_CheckBox(&s_PageButton3, Localize(""), s_Page == 3, &Button))
		s_Page = 3;

	Temp.VSplitLeft(8.0f, 0, &Temp);
	Temp.VSplitLeft(14.0f, &Button, &Temp);
	if (DoButton_CheckBox(&s_PageButton4, Localize(""), s_Page == 4, &Button))
		s_Page = 4;

	MainView.Margin(10.0f, &MainView);
	RenderTools()->DrawUIRect(&MainView, vec4(1, 1, 1, 0.25f), CUI::CORNER_ALL, 10.0f);


	if (s_Page == 1)
	{

	}
	if (s_Page == 2)
	{

	}
	if (s_Page == 3)
	{

	}
	if (s_Page == 4)
	{

	}
}

void CMenus::OnInit()
{

	/*
	array<string> my_strings;
	array<string>::range r2;
	my_strings.add("4");
	my_strings.add("6");
	my_strings.add("1");
	my_strings.add("3");
	my_strings.add("7");
	my_strings.add("5");
	my_strings.add("2");

	for(array<string>::range r = my_strings.all(); !r.empty(); r.pop_front())
		dbg_msg("", "%s", r.front().cstr());

	sort(my_strings.all());

	dbg_msg("", "after:");
	for(array<string>::range r = my_strings.all(); !r.empty(); r.pop_front())
		dbg_msg("", "%s", r.front().cstr());


	array<int> myarray;
	myarray.add(4);
	myarray.add(6);
	myarray.add(1);
	myarray.add(3);
	myarray.add(7);
	myarray.add(5);
	myarray.add(2);

	for(array<int>::range r = myarray.all(); !r.empty(); r.pop_front())
		dbg_msg("", "%d", r.front());

	sort(myarray.all());
	sort_verify(myarray.all());

	dbg_msg("", "after:");
	for(array<int>::range r = myarray.all(); !r.empty(); r.pop_front())
		dbg_msg("", "%d", r.front());

	exit(-1);
	// */

	if(g_Config.m_ClShowWelcome)
		m_Popup = POPUP_LANGUAGE;
	g_Config.m_ClShowWelcome = 0;

	Console()->Chain("add_favorite", ConchainServerbrowserUpdate, this);
	Console()->Chain("remove_favorite", ConchainServerbrowserUpdate, this);
	Console()->Chain("add_friend", ConchainFriendlistUpdate, this);
	Console()->Chain("remove_friend", ConchainFriendlistUpdate, this);

	// setup load amount
	m_LoadCurrent = 0;
	m_LoadTotal = g_pData->m_NumImages;
	if(!g_Config.m_ClThreadsoundloading)
		m_LoadTotal += g_pData->m_NumSounds;
}

void CMenus::PopupMessage(const char *pTopic, const char *pBody, const char *pButton)
{
	// reset active item
	UI()->SetActiveItem(0);

	str_copy(m_aMessageTopic, pTopic, sizeof(m_aMessageTopic));
	str_copy(m_aMessageBody, pBody, sizeof(m_aMessageBody));
	str_copy(m_aMessageButton, pButton, sizeof(m_aMessageButton));
	m_Popup = POPUP_MESSAGE;
}

void CMenus::RenderInfo(CUIRect MainView)
{
	const float MaxWidth = 180.0f, MinWidth = 60.0f / UI()->Scale();

	static int64 s_LastFade = 0;
	static float BoxWidth = MinWidth;
	static int s_BoxAccount = 0;
	static bool s_FadeIn = false;


	CUIRect Frame, Button, Label;
	int64 Now = time_get();
	char aBuf[512];
	int Auth = Client()->XGetAuth();
	int acc = 100;
	float t = acc / 0.5f;
	float a = 0;
	float FontSize = 12.0f*UI()->Scale();
	bool ForceFade = false;

	// version info
	MainView.Margin(4.0f, &Button);
	Button.HSplitTop(8.0f, 0, &Label);
	Label.VMargin(16.0f, &Label);
	if(str_comp(Client()->LatestVersion(), "0") != 0)
	{
		str_format(aBuf, sizeof(aBuf), Localize("Teeworlds %s is out! Download it at www.teeworlds.com!"), Client()->LatestVersion());
		TextRender()->TextColor(1.0f, 0.4f, 0.4f, 1.0f);
		UI()->DoLabelScaled(&Label, aBuf, FontSize, 1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), GAME_VERSION);
		UI()->DoLabelScaled(&Label, aBuf, FontSize, 1);
		Label.VSplitRight(30.0f, &Label, 0);
		str_format(aBuf, sizeof(aBuf), Localize("Teeworlds version: "));
		UI()->DoLabelScaled(&Label, aBuf, FontSize, 1);

	}

	Button.HSplitTop(10.0f+FontSize, 0, &Label);
	Label.VMargin(16.0f, &Label);
	if(Client()->XGetVersion()[0] != 0 && str_comp(Client()->XGetVersion(), "MATCH") != 0) //TODO
	{
		str_format(aBuf, sizeof(aBuf), Localize("xClient %s is out! Download it at www.13x37.com!"), Client()->XGetVersion());
		TextRender()->TextColor(1.0f, 0.4f, 0.4f, 1.0f);
		UI()->DoLabelScaled(&Label, aBuf, FontSize, 1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), X_VERSION);
		UI()->DoLabelScaled(&Label, aBuf, FontSize, 1);
		Label.VSplitRight(30.0f, &Label, 0);
		str_format(aBuf, sizeof(aBuf), Localize("xClient version: "));
		UI()->DoLabelScaled(&Label, aBuf, FontSize, 1);
	}


	// login info
	//force fade in for 3 seconds after login
	if(Now - ms_LastLogin <= time_freq() * 3)
	{
		ForceFade = true;
		s_LastFade = Now;
		BoxWidth = MaxWidth;
	}

	MainView.HMargin(5.0f, &Frame);
	//Frame.HSplitTop(48.0f / UI()->Scale(), &Frame, 0);
	Frame.VSplitLeft(BoxWidth, &Frame, 0);

	RenderTools()->DrawUIRect(&Frame, vec4(0, 0, 0, 0.5f), CUI::CORNER_R, 12.0f);

	if(UI()->DoButtonLogic(&s_BoxAccount, "", 0, &Frame))
	{
		if(!Client()->XGetAuth())
			m_Popup = POPUP_LOGIN;
	}

	if(UI()->MouseInside(&Frame))
		str_format(m_aToolTip, sizeof(m_aToolTip), "Log into your 13x37 account to access special features");

	if(UI()->MouseInside(&Frame) && !s_FadeIn && BoxWidth == MinWidth)
	{
		s_FadeIn = true;
		s_LastFade = Now;
	}
	else if(!UI()->MouseInside(&Frame) && s_FadeIn && BoxWidth == MaxWidth)
	{
		s_FadeIn = false;
		s_LastFade = Now;
	}
	if(s_FadeIn)
	{
		a = 1.0f - ((float)clamp(acc - (int)((Now - s_LastFade) / (time_freq() / t)), 0, acc) / acc);
		BoxWidth = MinWidth + (MaxWidth - MinWidth)*a;
	}
	else
	{
		a = ((float)clamp(acc - (int)((Now - s_LastFade) / (time_freq() / t)), 0, acc) / acc);
		BoxWidth = MinWidth + (MaxWidth - MinWidth)*a;
	}

	if(ForceFade)
		BoxWidth = MaxWidth;


	Frame.Margin(5.0f, &Frame);
	Frame.VSplitLeft(40.0f / UI()->Scale(), &Button, &Frame);
	RenderTools()->DrawUIRect(&Button, vec4(0,0,0,0.5f), CUI::CORNER_ALL, 0.0f);
	RenderTools()->DrawUIRect(&Button, vec4(1, 1, 1, 1), 0, 0, Auth ? gs_TextureAvaDefault : gs_TextureAvaOffline);

	UI()->ClipEnable(&Frame);
	Frame.VMargin(5.0f, &Frame);
	Frame.x += BoxWidth - MaxWidth;

	
	if(Auth)
	{
		if(ForceFade)
			str_format(aBuf, sizeof(aBuf), "Logged in as %s", Auth == 3 ? "admin" : Auth == 2 ? "vip" : Auth == 1 ? "user" : "idk lol");
		else
			str_format(aBuf, sizeof(aBuf), "Online");
		Frame.HSplitTop(16.f / UI()->Scale(), &Button, 0);
		TextRender()->TextColor(0.5f, 1.0f, 0.5f, 1.0f);
		UI()->DoLabelScaled(&Button, aBuf, FontSize, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
		Frame.HSplitBottom(14.f / UI()->Scale(), 0, &Button);
		UI()->DoLabelScaled(&Button, Client()->XGetName(), FontSize, -1);
	}
	else
	{
		if(ForceFade)
			str_format(aBuf, sizeof(aBuf), "Failed to login");
		else
			str_format(aBuf, sizeof(aBuf), "Offline");
		Frame.HSplitTop(16.f / UI()->Scale(), &Button, &Frame);
		TextRender()->TextColor(1.0f, 0.5f, 0.5f, 1.0f);
		UI()->DoLabelScaled(&Button, aBuf, FontSize, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

		if(ForceFade)
			str_format(aBuf, sizeof(aBuf), Client()->XGetName());
		else
			str_format(aBuf, sizeof(aBuf), "Click to login");
		Frame.HSplitBottom(14.f / UI()->Scale(), 0, &Button);
		UI()->DoLabelScaled(&Button, aBuf, FontSize, -1);
	}

	UI()->ClipDisable();

}

//void CMenus::RenderAccount(CUIRect MainView)
//{
//	CUIRect LeftView, RightView;
//	CUIRect Button, Label, Temp;
//	char aBuf[128];
//
//	MainView.VMargin(30.0f, &MainView);
//	Temp = MainView;
//	Temp.h *= 2.5f;
//	RenderTools()->DrawUIRect(&Temp, ms_ColorTabbarActive, CUI::CORNER_T, 30.0f);
//	MainView.Margin(2.0f, &MainView);
//	MainView.VMargin(30.0f, &MainView);
//	
//	// Teeworlds version
//	MainView.HSplitTop(10.0f, &Label, &Temp);
//	if (str_comp(Client()->LatestVersion(), "0") != 0)
//	{
//		str_format(aBuf, sizeof(aBuf), Localize("Teeworlds %s is out! Download it at www.teeworlds.com!"), Client()->LatestVersion());
//		TextRender()->TextColor(1.0f, 0.4f, 0.4f, 1.0f);
//	}
//	else
//		str_format(aBuf, sizeof(aBuf), Localize("Teeworlds version: %s"), GAME_VERSION);
//	UI()->DoLabelScaled(&Label, aBuf, 10.0f, 1);
//	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
//
//	// xClient version
//	Temp.HSplitTop(1.0f, 0, &Temp);
//	str_format(aBuf, sizeof(aBuf), Localize("xClient version: %s"), X_VERSION);
//	UI()->DoLabelScaled(&Temp, aBuf, 10.0f, 1);
//	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
//
//	MainView.Margin(2.0f, &MainView);
//
//	if (1 == 2) // offline
//	{
//		// Nick
//		MainView.VSplitLeft(60.0f, &Label, &MainView);
//		Label.HSplitTop(4.0f, 0, &Label);
//		UI()->DoLabel(&Label, Localize("Nickname"), 10.0f, -1);
//		MainView.VSplitLeft(100.0f, &Button, &MainView);
//		static float OffsetName = 0.0f;
//		Button.HMargin(2.0f, &Button);
//		DoEditBox(&g_Config.m_XLoginName, &Button, g_Config.m_XLoginName, sizeof(g_Config.m_XLoginName), 10.0f, &OffsetName);
//
//		// Pass
//		MainView.VSplitLeft(10.0f, 0, &MainView);
//		MainView.VSplitLeft(60.0f, &Label, &MainView);
//		Label.HSplitTop(4.0f, 0, &Label);
//		UI()->DoLabel(&Label, Localize("Password"), 10.0f, -1);
//		MainView.VSplitLeft(100.0f, &Button, &MainView);
//		static float OffsetPass = 0.0f;
//		Button.HMargin(2.0f, &Button);
//		DoEditBox(&g_Config.m_XLoginPass, &Button, g_Config.m_XLoginPass, sizeof(g_Config.m_XLoginPass), 10.0f, &OffsetPass, true);
//
//		// Login button
//		MainView.VSplitLeft(10.0f, 0, &MainView);
//		MainView.VSplitLeft(60.0f, &Button, &MainView);
//		static int s_ButtonLogin = 0;
//		Button.HMargin(1.0f, &Button);
//		if (DoButton_Menu(&s_ButtonLogin, Localize("Login"), 0, &Button))
//		{
//			// TODO: login
//		}
//	}
//	else // online
//	{
//		float Width = 0;
//		
//		// Nick
//		// Pass
//		str_format(aBuf, sizeof(aBuf), "%s | %s", "Shadd!X", "Administrator");
//		Width = 24.0f + TextRender()->TextWidth(0, 10.0f, aBuf, -1);
//		MainView.VSplitLeft(Width, &Label, &MainView);
//		Label.VSplitLeft(Width, &Label, 0);
//		RenderTools()->DrawUIRect(&Label, ms_ColorTabbarActive, CUI::CORNER_ALL, 6.0f);
//		Label.VSplitLeft(14.0f, &Temp, &Label);
//		RenderTools()->DrawUIRect(&Temp, vec4(1, 1, 1, 0.5f), CUI::CORNER_L, 6.0f);
//		Label.HSplitTop(4.0f, 0, &Label);
//		UI()->DoLabel(&Label, aBuf, 10.0f, 0);
//		Temp.HSplitTop(4.0f, 0, &Temp);
//		UI()->DoLabel(&Temp, "3", 10.0f, 0);
//
//
//		
//		// Pass
//		//str_format(aBuf, sizeof(aBuf), "%s", "Administrator");
//		//Width = 24.0f + TextRender()->TextWidth(0, 10.0f, aBuf, -1);
//		//MainView.VSplitLeft(10.0f, 0, &MainView);
//		//MainView.VSplitLeft(Width, &Label, &MainView);
//		//Label.VSplitLeft(Width, &Label, 0);
//		//RenderTools()->DrawUIRect(&Label, ms_ColorTabbarActive, CUI::CORNER_ALL, 6.0f);
//		//Label.VSplitLeft(14.0f, &Temp, &Label);
//		//RenderTools()->DrawUIRect(&Temp, vec4(1,1,1,0.5f), CUI::CORNER_L, 6.0f);
//		//Label.HSplitTop(4.0f, 0, &Label);
//		//TextRender()->TextColor(0.6f, 0.0f, 1.0f, 1.0f);
//		//UI()->DoLabel(&Label, aBuf , 10.0f, 0);
//		//TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
//		//Temp.HSplitTop(4.0f, 0, &Temp);
//		//UI()->DoLabel(&Temp, "3", 10.0f, 0);
//
//	}
//}

int CMenus::Render()
{
	CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	static bool s_First = true;
	if(s_First)
	{
		if(g_Config.m_UiPage == PAGE_BROWSER)
			ServerBrowser()->Refresh(IServerBrowser::TYPE_INTERNET);
		m_pClient->m_pSounds->Enqueue(CSounds::CHN_MUSIC, SOUND_MENU);
		s_First = false;
	}

	if(Client()->State() == IClient::STATE_ONLINE)
	{
		ms_ColorTabbarInactive = ms_ColorTabbarInactiveIngame;
		ms_ColorTabbarActive = ms_ColorTabbarActiveIngame;
	}
	else
	{
		RenderBackground();
		ms_ColorTabbarInactive = ms_ColorTabbarInactiveOutgame;
		ms_ColorTabbarActive = ms_ColorTabbarActiveOutgame;
	}

	CUIRect TabBar;
	CUIRect ToolTip;
	CUIRect InfoBar;
	CUIRect MainView;

	if (m_Popup == POPUP_NONE)
	{
		Screen.HSplitBottom(134.0f, 0, &TabBar);
		TabBar.HSplitBottom(68.0f, &TabBar, &InfoBar);
		RenderTools()->DrawUIRect(&TabBar, vec4(0.0f, 0.0f, 0.0f, 0.5f), 0, 0.0f);

		InfoBar.HSplitTop(20.0f, 0, &InfoBar);
	}

	// some margin around the screen
	Screen.Margin(10.0f, &Screen);

	static bool s_SoundCheck = false;
	if(!s_SoundCheck && m_Popup == POPUP_NONE)
	{
		if(Client()->SoundInitFailed())
			m_Popup = POPUP_SOUNDERROR;
		s_SoundCheck = true;
	}

	if(m_Popup == POPUP_NONE)
	{
		// do tab bar
		Screen.HSplitBottom(132.0f, &MainView, &TabBar);
		MainView.HSplitBottom(16.0f, &MainView, &ToolTip);
		TabBar.VMargin(20.0f, &TabBar);
		TabBar.HSplitBottom(30.0f, &TabBar, 0);

		RenderMenubar(TabBar);

		/*RenderTools()->DrawUIRect(&MainView, vec4(1, 0, 0, 1), CUI::CORNER_ALL, 6.0f);
		RenderTools()->DrawUIRect(&TabBar, vec4(1, 1, 0, 1), CUI::CORNER_ALL, 6.0f);
		RenderTools()->DrawUIRect(&ToolTip, vec4(0, 1, 0, 1), CUI::CORNER_ALL, 6.0f);
		RenderTools()->DrawUIRect(&InfoBar, vec4(0, 0, 1, 1), CUI::CORNER_ALL, 6.0f);*/

		// do tooltip
		
		if(m_aToolTip[0] != 0)
		{
			//RenderTools()->DrawUIRect(&ToolTip, ms_ColorTabbarActive, CUI::CORNER_ALL, 6.0f);
			ToolTip.h += 8.0f;
			ToolTip.Margin(3.0f, &ToolTip);
			UI()->DoLabel(&ToolTip, m_aToolTip, ToolTip.h*ms_FontmodHeight, 0);
			m_aToolTip[0] = 0;
		}

		// account
		RenderInfo(InfoBar);

		if (g_Config.m_UiPage < PAGE_NONE || g_Config.m_UiPage > PAGE_SETTINGS /*|| (Client()->State() == IClient::STATE_OFFLINE && g_Config.m_UiPage >= PAGE_GAME && g_Config.m_UiPage <= PAGE_CALLVOTE)*/)
		{
			//ServerBrowser()->Refresh(IServerBrowser::TYPE_INTERNET);
			g_Config.m_UiPage = PAGE_NONE;
		}

		// render current page
		if(Client()->State() != IClient::STATE_OFFLINE)
		{
			if (m_GamePage == PAGE_SERVER)
			{
				CUIRect Tabbar;
				MainView.HSplitTop(24.0f, &Tabbar, &MainView);
				RenderServer(Tabbar);

				if (m_ServerPage == SERVER_GAME)
					RenderGame(MainView);
				else if (m_ServerPage == SERVER_PLAYERS)
					RenderPlayers(MainView);
				else if (m_ServerPage == SERVER_SERVER_INFO)
					RenderServerInfo(MainView);
				else if (m_ServerPage == SERVER_CALLVOTE)
					RenderServerControl(MainView);
			}
				
			else if (m_GamePage == PAGE_NEWS)
				RenderNews(MainView);
			else if (m_GamePage == PAGE_BROWSER)
				RenderServerbrowser(MainView);
			else if (m_GamePage == PAGE_DEMOS)
				RenderDemoList(MainView);
			else if(m_GamePage == PAGE_SETTINGS)
				RenderSettings(MainView);
			else if(m_GamePage == PAGE_13X37)
				Render13x37(MainView);
		}
		else if(g_Config.m_UiPage == PAGE_NEWS)
			RenderNews(MainView);
		else if(g_Config.m_UiPage == PAGE_BROWSER)
			RenderServerbrowser(MainView);
		else if(g_Config.m_UiPage == PAGE_DEMOS)
			RenderDemoList(MainView);
		else if(g_Config.m_UiPage == PAGE_SETTINGS)
			RenderSettings(MainView);
		else if(g_Config.m_UiPage == PAGE_13X37)
			Render13x37(MainView);
	}
	else
	{
		// make sure that other windows doesn't do anything funnay!
		//UI()->SetHotItem(0);
		//UI()->SetActiveItem(0);
		char aBuf[128];
		const char *pTitle = "";
		const char *pExtraText = "";
		const char *pButtonText = "";
		int ExtraAlign = 0;

		if(m_Popup == POPUP_MESSAGE)
		{
			pTitle = m_aMessageTopic;
			pExtraText = m_aMessageBody;
			pButtonText = m_aMessageButton;
		}
		else if(m_Popup == POPUP_CONNECTING)
		{
			pTitle = Localize("Connecting to");
			pExtraText = g_Config.m_UiServerAddress; // TODO: query the client about the address
			pButtonText = Localize("Abort");
			if(Client()->MapDownloadTotalsize() > 0)
			{
				pTitle = Localize("Downloading map");
				pExtraText = "";
			}
		}
		else if(m_Popup == POPUP_DISCONNECTED)
		{
			pTitle = Localize("Disconnected");
			pExtraText = Client()->ErrorString();
			pButtonText = Localize("Ok");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_PURE)
		{
			pTitle = Localize("Disconnected");
			pExtraText = Localize("The server is running a non-standard tuning on a pure game type.");
			pButtonText = Localize("Ok");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_DELETE_DEMO)
		{
			pTitle = Localize("Delete demo");
			pExtraText = Localize("Are you sure that you want to delete the demo?");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_RENAME_DEMO)
		{
			pTitle = Localize("Rename demo");
			pExtraText = "";
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_REMOVE_FRIEND)
		{
			pTitle = Localize("Remove friend");
			pExtraText = Localize("Are you sure that you want to remove the player from your friends list?");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_SOUNDERROR)
		{
			pTitle = Localize("Sound error");
			pExtraText = Localize("The audio device couldn't be initialised.");
			pButtonText = Localize("Ok");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_PASSWORD)
		{
			pTitle = Localize("Password incorrect");
			pExtraText = "";
			pButtonText = Localize("Try again");
		}
		else if(m_Popup == POPUP_LOGIN)
		{
			pTitle = Localize("Login");
			pExtraText = "";
			pButtonText = Localize("Try again");
		}
		else if(m_Popup == POPUP_QUIT)
		{
			pTitle = Localize("Quit");
			pExtraText = Localize("Are you sure that you want to quit?");
			ExtraAlign = -1;
		}
		else if(m_Popup == POPUP_FIRST_LAUNCH)
		{
			pTitle = Localize("Welcome to Teeworlds");
			pExtraText = Localize("As this is the first time you launch the game, please enter your nick name below. It's recommended that you check the settings to adjust them to your liking before joining a server.");
			pButtonText = Localize("Ok");
			ExtraAlign = -1;
		}

		CUIRect Box, Part;
		Box = Screen;
		Box.VMargin(150.0f/UI()->Scale(), &Box);
		Box.HMargin(150.0f/UI()->Scale(), &Box);

		// render the box
		RenderTools()->DrawUIRect(&Box, vec4(0,0,0,0.5f), CUI::CORNER_ALL, 15.0f);

		Box.HSplitTop(20.f/UI()->Scale(), &Part, &Box);
		Box.HSplitTop(24.f/UI()->Scale(), &Part, &Box);
		UI()->DoLabelScaled(&Part, pTitle, 24.f, 0);
		Box.HSplitTop(20.f/UI()->Scale(), &Part, &Box);
		Box.HSplitTop(24.f/UI()->Scale(), &Part, &Box);
		Part.VMargin(20.f/UI()->Scale(), &Part);

		if(ExtraAlign == -1)
			UI()->DoLabelScaled(&Part, pExtraText, 20.f, -1, (int)Part.w);
		else
			UI()->DoLabelScaled(&Part, pExtraText, 20.f, 0, -1);

		if(m_Popup == POPUP_QUIT)
		{
			CUIRect Yes, No;
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);

			// additional info
			Box.HSplitTop(10.0f, 0, &Box);
			Box.VMargin(20.f/UI()->Scale(), &Box);
			if(m_pClient->Editor()->HasUnsavedData())
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "%s\n%s", Localize("There's an unsaved map in the editor, you might want to save it before you quit the game."), Localize("Quit anyway?"));
				UI()->DoLabelScaled(&Box, aBuf, 20.f, -1, Part.w-20.0f);
			}

			// buttons
			Part.VMargin(80.0f, &Part);
			Part.VSplitMid(&No, &Yes);
			Yes.VMargin(20.0f, &Yes);
			No.VMargin(20.0f, &No);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("No"), 0, &No) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonTryAgain = 0;
			if(DoButton_Menu(&s_ButtonTryAgain, Localize("Yes"), 0, &Yes) || m_EnterPressed)
				Client()->Quit();
		}
		else if(m_Popup == POPUP_PASSWORD)
		{
			CUIRect Label, TextBox, TryAgain, Abort;

			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			Part.VSplitMid(&Abort, &TryAgain);

			TryAgain.VMargin(20.0f, &TryAgain);
			Abort.VMargin(20.0f, &Abort);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("Abort"), 0, &Abort) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonTryAgain = 0;
			if(DoButton_Menu(&s_ButtonTryAgain, Localize("Try again"), 0, &TryAgain) || m_EnterPressed)
			{
				Client()->Connect(g_Config.m_UiServerAddress);
			}

			Box.HSplitBottom(60.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);

			Part.VSplitLeft(60.0f, 0, &Label);
			Label.VSplitLeft(100.0f, 0, &TextBox);
			TextBox.VSplitLeft(20.0f, 0, &TextBox);
			TextBox.VSplitRight(60.0f, &TextBox, 0);
			UI()->DoLabel(&Label, Localize("Password"), 18.0f, -1);
			static float Offset = 0.0f;
			DoEditBox(&g_Config.m_Password, &TextBox, g_Config.m_Password, sizeof(g_Config.m_Password), 12.0f, &Offset, true);
		}
		else if(m_Popup == POPUP_LOGIN)
		{
			CUIRect Label, TextBox, TryAgain, Abort;

			static char aName[64], aPass[64];

			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			Part.VSplitMid(&Abort, &TryAgain);

			TryAgain.VMargin(20.0f, &TryAgain);
			Abort.VMargin(20.0f, &Abort);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("Abort"), 0, &Abort) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonTryAgain = 0;
			if(DoButton_Menu(&s_ButtonTryAgain, Localize("Login"), 0, &TryAgain) || m_EnterPressed)
			{
				ms_LastLogin = time_get();
				m_Popup = POPUP_NONE;
				IClient::CLoginInfo *pInfo = new IClient::CLoginInfo();
				str_format(pInfo->m_Name, sizeof(pInfo->m_Name), aName);
				str_format(pInfo->m_Pass, sizeof(pInfo->m_Pass), aPass);
				Client()->XSendRequest(IClient::REQUEST_AUTH, pInfo);
			}

			Box.HSplitBottom(80.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);

			Part.VSplitLeft(60.0f, 0, &Label);
			Label.VSplitLeft(100.0f, 0, &TextBox);
			TextBox.VSplitLeft(20.0f, 0, &TextBox);
			TextBox.VSplitRight(60.0f, &TextBox, 0);
			UI()->DoLabel(&Label, Localize("Password"), 18.0f, -1);
			static float Offset = 0.0f;
			DoEditBox(&aPass, &TextBox, aPass, 32, 12.0f, &Offset, true);



			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);

			Part.VSplitLeft(60.0f, 0, &Label);
			Label.VSplitLeft(100.0f, 0, &TextBox);
			TextBox.VSplitLeft(20.0f, 0, &TextBox);
			TextBox.VSplitRight(60.0f, &TextBox, 0);
			UI()->DoLabel(&Label, Localize("Name"), 18.0f, -1);
			DoEditBox(&aName, &TextBox, aName, 32, 12.0f, &Offset, false);
		}
		else if(m_Popup == POPUP_CONNECTING)
		{
			Box = Screen;
			Box.VMargin(150.0f, &Box);
			Box.HMargin(150.0f, &Box);
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(120.0f, &Part);

			static int s_Button = 0;
			if(DoButton_Menu(&s_Button, pButtonText, 0, &Part) || m_EscapePressed || m_EnterPressed)
			{
				Client()->Disconnect();
				m_Popup = POPUP_NONE;
			}

			if(Client()->MapDownloadTotalsize() > 0)
			{
				int64 Now = time_get();
				if(Now-m_DownloadLastCheckTime >= time_freq())
				{
					if(m_DownloadLastCheckSize > Client()->MapDownloadAmount())
					{
						// map downloaded restarted
						m_DownloadLastCheckSize = 0;
					}

					// update download speed
					float Diff = (Client()->MapDownloadAmount()-m_DownloadLastCheckSize)/((int)((Now-m_DownloadLastCheckTime)/time_freq()));
					float StartDiff = m_DownloadLastCheckSize-0.0f;
					if(StartDiff+Diff > 0.0f)
						m_DownloadSpeed = (Diff/(StartDiff+Diff))*(Diff/1.0f) + (StartDiff/(Diff+StartDiff))*m_DownloadSpeed;
					else
						m_DownloadSpeed = 0.0f;
					m_DownloadLastCheckTime = Now;
					m_DownloadLastCheckSize = Client()->MapDownloadAmount();
				}

				Box.HSplitTop(64.f, 0, &Box);
				Box.HSplitTop(24.f, &Part, &Box);
				str_format(aBuf, sizeof(aBuf), "%d/%d KiB (%.1f KiB/s)", Client()->MapDownloadAmount()/1024, Client()->MapDownloadTotalsize()/1024,	m_DownloadSpeed/1024.0f);
				UI()->DoLabel(&Part, aBuf, 20.f, 0, -1);

				// time left
				const char *pTimeLeftString;
				int TimeLeft = max(1, m_DownloadSpeed > 0.0f ? static_cast<int>((Client()->MapDownloadTotalsize()-Client()->MapDownloadAmount())/m_DownloadSpeed) : 1);
				if(TimeLeft >= 60)
				{
					TimeLeft /= 60;
					pTimeLeftString = TimeLeft == 1 ? Localize("%i minute left") : Localize("%i minutes left");
				}
				else
					pTimeLeftString = TimeLeft == 1 ? Localize("%i second left") : Localize("%i seconds left");
				Box.HSplitTop(20.f, 0, &Box);
				Box.HSplitTop(24.f, &Part, &Box);
				str_format(aBuf, sizeof(aBuf), pTimeLeftString, TimeLeft);
				UI()->DoLabel(&Part, aBuf, 20.f, 0, -1);

				// progress bar
				Box.HSplitTop(20.f, 0, &Box);
				Box.HSplitTop(24.f, &Part, &Box);
				Part.VMargin(40.0f, &Part);
				RenderTools()->DrawUIRect(&Part, vec4(1.0f, 1.0f, 1.0f, 0.25f), CUI::CORNER_ALL, 5.0f);
				Part.w = max(10.0f, (Part.w*Client()->MapDownloadAmount())/Client()->MapDownloadTotalsize());
				RenderTools()->DrawUIRect(&Part, vec4(1.0f, 1.0f, 1.0f, 0.5f), CUI::CORNER_ALL, 5.0f);
			}
		}
		else if(m_Popup == POPUP_LANGUAGE)
		{
			Box = Screen;
			Box.VMargin(150.0f, &Box);
			Box.HMargin(150.0f, &Box);
			Box.HSplitTop(20.f, &Part, &Box);
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Box.HSplitBottom(20.f, &Box, 0);
			Box.VMargin(20.0f, &Box);
			RenderLanguageSelection(Box);
			Part.VMargin(120.0f, &Part);

			static int s_Button = 0;
			if(DoButton_Menu(&s_Button, Localize("Ok"), 0, &Part) || m_EscapePressed || m_EnterPressed)
				m_Popup = POPUP_FIRST_LAUNCH;
		}
		else if(m_Popup == POPUP_COUNTRY)
		{
			Box = Screen;
			Box.VMargin(150.0f, &Box);
			Box.HMargin(150.0f, &Box);
			Box.HSplitTop(20.f, &Part, &Box);
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Box.HSplitBottom(20.f, &Box, 0);
			Box.VMargin(20.0f, &Box);

			static int ActSelection = -2;
			if(ActSelection == -2)
				ActSelection = g_Config.m_BrFilterCountryIndex;
			static float s_ScrollValue = 0.0f;
			int OldSelected = -1;
			UiDoListboxStart(&s_ScrollValue, &Box, 50.0f, Localize("Country"), "", m_pClient->m_pCountryFlags->Num(), 6, OldSelected, s_ScrollValue);

			for(int i = 0; i < m_pClient->m_pCountryFlags->Num(); ++i)
			{
				const CCountryFlags::CCountryFlag *pEntry = m_pClient->m_pCountryFlags->GetByIndex(i);
				if(pEntry->m_CountryCode == ActSelection)
					OldSelected = i;

				CListboxItem Item = UiDoListboxNextItem(&pEntry->m_CountryCode, OldSelected == i);
				if(Item.m_Visible)
				{
					CUIRect Label;
					Item.m_Rect.Margin(5.0f, &Item.m_Rect);
					Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Label);
					float OldWidth = Item.m_Rect.w;
					Item.m_Rect.w = Item.m_Rect.h*2;
					Item.m_Rect.x += (OldWidth-Item.m_Rect.w)/ 2.0f;
					vec4 Color(1.0f, 1.0f, 1.0f, 1.0f);
					m_pClient->m_pCountryFlags->Render(pEntry->m_CountryCode, &Color, Item.m_Rect.x, Item.m_Rect.y, Item.m_Rect.w, Item.m_Rect.h);
					UI()->DoLabel(&Label, pEntry->m_aCountryCodeString, 10.0f, 0);
				}
			}

			const int NewSelected = UiDoListboxEnd(&s_ScrollValue, 0);
			if(OldSelected != NewSelected)
				ActSelection = m_pClient->m_pCountryFlags->GetByIndex(NewSelected)->m_CountryCode;

			Part.VMargin(120.0f, &Part);

			static int s_Button = 0;
			if(DoButton_Menu(&s_Button, Localize("Ok"), 0, &Part) || m_EnterPressed)
			{
				g_Config.m_BrFilterCountryIndex = ActSelection;
				Client()->ServerBrowserUpdate();
				m_Popup = POPUP_NONE;
			}

			if(m_EscapePressed)
			{
				ActSelection = g_Config.m_BrFilterCountryIndex;
				m_Popup = POPUP_NONE;
			}
		}
		else if(m_Popup == POPUP_DELETE_DEMO)
		{
			CUIRect Yes, No;
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			Part.VSplitMid(&No, &Yes);

			Yes.VMargin(20.0f, &Yes);
			No.VMargin(20.0f, &No);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("No"), 0, &No) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonTryAgain = 0;
			if(DoButton_Menu(&s_ButtonTryAgain, Localize("Yes"), 0, &Yes) || m_EnterPressed)
			{
				m_Popup = POPUP_NONE;
				// delete demo
				if(m_DemolistSelectedIndex >= 0 && !m_DemolistSelectedIsDir)
				{
					char aBuf[512];
					str_format(aBuf, sizeof(aBuf), "%s/%s", m_aCurrentDemoFolder, m_lDemos[m_DemolistSelectedIndex].m_aFilename);
					if(Storage()->RemoveFile(aBuf, m_lDemos[m_DemolistSelectedIndex].m_StorageType))
					{
						DemolistPopulate();
						DemolistOnUpdate(false);
					}
					else
						PopupMessage(Localize("Error"), Localize("Unable to delete the demo"), Localize("Ok"));
				}
			}
		}
		else if(m_Popup == POPUP_RENAME_DEMO)
		{
			CUIRect Label, TextBox, Ok, Abort;

			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			Part.VSplitMid(&Abort, &Ok);

			Ok.VMargin(20.0f, &Ok);
			Abort.VMargin(20.0f, &Abort);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("Abort"), 0, &Abort) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonOk = 0;
			if(DoButton_Menu(&s_ButtonOk, Localize("Ok"), 0, &Ok) || m_EnterPressed)
			{
				m_Popup = POPUP_NONE;
				// rename demo
				if(m_DemolistSelectedIndex >= 0 && !m_DemolistSelectedIsDir)
				{
					char aBufOld[512];
					str_format(aBufOld, sizeof(aBufOld), "%s/%s", m_aCurrentDemoFolder, m_lDemos[m_DemolistSelectedIndex].m_aFilename);
					int Length = str_length(m_aCurrentDemoFile);
					char aBufNew[512];
					if(Length <= 4 || m_aCurrentDemoFile[Length-5] != '.' || str_comp_nocase(m_aCurrentDemoFile+Length-4, "demo"))
						str_format(aBufNew, sizeof(aBufNew), "%s/%s.demo", m_aCurrentDemoFolder, m_aCurrentDemoFile);
					else
						str_format(aBufNew, sizeof(aBufNew), "%s/%s", m_aCurrentDemoFolder, m_aCurrentDemoFile);
					if(Storage()->RenameFile(aBufOld, aBufNew, m_lDemos[m_DemolistSelectedIndex].m_StorageType))
					{
						DemolistPopulate();
						DemolistOnUpdate(false);
					}
					else
						PopupMessage(Localize("Error"), Localize("Unable to rename the demo"), Localize("Ok"));
				}
			}

			Box.HSplitBottom(60.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);

			Part.VSplitLeft(60.0f, 0, &Label);
			Label.VSplitLeft(120.0f, 0, &TextBox);
			TextBox.VSplitLeft(20.0f, 0, &TextBox);
			TextBox.VSplitRight(60.0f, &TextBox, 0);
			UI()->DoLabel(&Label, Localize("New name:"), 18.0f, -1);
			static float Offset = 0.0f;
			DoEditBox(&Offset, &TextBox, m_aCurrentDemoFile, sizeof(m_aCurrentDemoFile), 12.0f, &Offset);
		}
		else if(m_Popup == POPUP_REMOVE_FRIEND)
		{
			CUIRect Yes, No;
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			Part.VSplitMid(&No, &Yes);

			Yes.VMargin(20.0f, &Yes);
			No.VMargin(20.0f, &No);

			static int s_ButtonAbort = 0;
			if(DoButton_Menu(&s_ButtonAbort, Localize("No"), 0, &No) || m_EscapePressed)
				m_Popup = POPUP_NONE;

			static int s_ButtonTryAgain = 0;
			if(DoButton_Menu(&s_ButtonTryAgain, Localize("Yes"), 0, &Yes) || m_EnterPressed)
			{
				m_Popup = POPUP_NONE;
				// remove friend
				if(m_FriendlistSelectedIndex >= 0)
				{
					m_pClient->Friends()->RemoveFriend(m_lFriends[m_FriendlistSelectedIndex].m_pFriendInfo->m_aName,
						m_lFriends[m_FriendlistSelectedIndex].m_pFriendInfo->m_aClan);
					FriendlistOnUpdate();
					Client()->ServerBrowserUpdate();
				}
			}
		}
		else if(m_Popup == POPUP_FIRST_LAUNCH)
		{
			CUIRect Label, TextBox;

			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(80.0f, &Part);

			static int s_EnterButton = 0;
			if(DoButton_Menu(&s_EnterButton, Localize("Enter"), 0, &Part) || m_EnterPressed)
				m_Popup = POPUP_NONE;

			Box.HSplitBottom(40.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);

			Part.VSplitLeft(60.0f, 0, &Label);
			Label.VSplitLeft(100.0f, 0, &TextBox);
			TextBox.VSplitLeft(20.0f, 0, &TextBox);
			TextBox.VSplitRight(60.0f, &TextBox, 0);
			UI()->DoLabel(&Label, Localize("Nickname"), 18.0f, -1);
			static float Offset = 0.0f;
			DoEditBox(&g_Config.m_PlayerName, &TextBox, g_Config.m_PlayerName, sizeof(g_Config.m_PlayerName), 12.0f, &Offset);
		}
		else
		{
			Box.HSplitBottom(20.f, &Box, &Part);
			Box.HSplitBottom(24.f, &Box, &Part);
			Part.VMargin(120.0f, &Part);

			static int s_Button = 0;
			if(DoButton_Menu(&s_Button, pButtonText, 0, &Part) || m_EscapePressed || m_EnterPressed)
				m_Popup = POPUP_NONE;
		}

		if(m_Popup == POPUP_NONE)
			UI()->SetActiveItem(0);
	}

	return 0;
}


void CMenus::SetActive(bool Active)
{
	m_MenuActive = Active;
	if(!m_MenuActive)
	{
		if(m_NeedSendinfo)
		{
			m_pClient->SendInfo(false);
			m_NeedSendinfo = false;
		}

		if(Client()->State() == IClient::STATE_ONLINE)
		{
			m_pClient->OnRelease();
		}
	}
	else if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		m_pClient->OnRelease();
	}
}

void CMenus::OnReset()
{
}

bool CMenus::OnMouseMove(float x, float y)
{
	m_LastInput = time_get();

	if(!m_MenuActive && !m_QAActive)
		return false;

	UI()->ConvertMouseMove(&x, &y);
	m_MousePos.x += x;
	m_MousePos.y += y;
	if(m_MousePos.x < 0) m_MousePos.x = 0;
	if(m_MousePos.y < 0) m_MousePos.y = 0;
	if(m_MousePos.x > Graphics()->ScreenWidth()) m_MousePos.x = Graphics()->ScreenWidth();
	if(m_MousePos.y > Graphics()->ScreenHeight()) m_MousePos.y = Graphics()->ScreenHeight();

	return true;
}

bool CMenus::OnInput(IInput::CEvent e)
{
	m_LastInput = time_get();

	// special handle esc and enter for popup purposes
	if(e.m_Flags&IInput::FLAG_PRESS)
	{
		if(e.m_Key == KEY_ESCAPE)
		{
			m_EscapePressed = true;
			SetActive(!IsActive());
			return true;
		}
	}

	if(IsActive())
	{
		if(e.m_Flags&IInput::FLAG_PRESS)
		{
			// special for popups
			if(e.m_Key == KEY_RETURN || e.m_Key == KEY_KP_ENTER)
				m_EnterPressed = true;
			else if(e.m_Key == KEY_DELETE)
				m_DeletePressed = true;
		}

		if(m_NumInputEvents < MAX_INPUTEVENTS)
			m_aInputEvents[m_NumInputEvents++] = e;
		return true;
	}
	return false;
}

void CMenus::OnStateChange(int NewState, int OldState)
{
	// reset active item
	UI()->SetActiveItem(0);

	if(NewState == IClient::STATE_OFFLINE)
	{
		if(OldState >= IClient::STATE_ONLINE && NewState < IClient::STATE_QUITING)
			m_pClient->m_pSounds->Play(CSounds::CHN_MUSIC, SOUND_MENU, 1.0f);
		m_Popup = POPUP_NONE;
		if(Client()->ErrorString() && Client()->ErrorString()[0] != 0)
		{
			if(str_find(Client()->ErrorString(), "password"))
			{
				m_Popup = POPUP_PASSWORD;
				UI()->SetHotItem(&g_Config.m_Password);
				UI()->SetActiveItem(&g_Config.m_Password);
			}
			else
				m_Popup = POPUP_DISCONNECTED;
		}
	}
	else if(NewState == IClient::STATE_LOADING)
	{
		m_Popup = POPUP_CONNECTING;
		m_DownloadLastCheckTime = time_get();
		m_DownloadLastCheckSize = 0;
		m_DownloadSpeed = 0.0f;
		//client_serverinfo_request();
	}
	else if(NewState == IClient::STATE_CONNECTING)
		m_Popup = POPUP_CONNECTING;
	else if (NewState == IClient::STATE_ONLINE || NewState == IClient::STATE_DEMOPLAYBACK)
	{
		m_Popup = POPUP_NONE;
		m_GamePage = PAGE_SERVER;
		SetActive(false);
	}
}

extern "C" void font_debug_render();

void CMenus::OnRender()
{
	/*
	// text rendering test stuff
	render_background();

	CTextCursor cursor;
	TextRender()->SetCursor(&cursor, 10, 10, 20, TEXTFLAG_RENDER);
	TextRender()->TextEx(&cursor, "ようこそ - ガイド", -1);

	TextRender()->SetCursor(&cursor, 10, 30, 15, TEXTFLAG_RENDER);
	TextRender()->TextEx(&cursor, "ようこそ - ガイド", -1);

	//Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->QuadsDrawTL(60, 60, 5000, 5000);
	Graphics()->QuadsEnd();
	return;*/

	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		SetActive(true);

	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		CUIRect Screen = *UI()->Screen();
		Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);
		RenderDemoPlayer(Screen);
	}

	if(Client()->State() == IClient::STATE_ONLINE && m_pClient->m_ServerMode == m_pClient->SERVERMODE_PUREMOD)
	{
		Client()->Disconnect();
		SetActive(true);
		m_Popup = POPUP_PURE;
	}

	if(!IsActive() && !m_QAActive)
	{
		m_EscapePressed = false;
		m_EnterPressed = false;
		m_DeletePressed = false;
		m_NumInputEvents = 0;
		return;
	}

	// update colors
	vec3 Rgb = HslToRgb(vec3(g_Config.m_UiColorHue/255.0f, g_Config.m_UiColorSat/255.0f, g_Config.m_UiColorLht/255.0f));
	ms_GuiColor = vec4(Rgb.r, Rgb.g, Rgb.b, g_Config.m_UiColorAlpha/255.0f);

	ms_ColorTabbarInactiveOutgame = vec4(0,0,0,0.25f);
	ms_ColorTabbarActiveOutgame = vec4(0,0,0,0.5f);

	float ColorIngameScaleI = 0.5f;
	float ColorIngameAcaleA = 0.2f;
	ms_ColorTabbarInactiveIngame = vec4(
		ms_GuiColor.r*ColorIngameScaleI,
		ms_GuiColor.g*ColorIngameScaleI,
		ms_GuiColor.b*ColorIngameScaleI,
		ms_GuiColor.a*0.8f);

	ms_ColorTabbarActiveIngame = vec4(
		ms_GuiColor.r*ColorIngameAcaleA,
		ms_GuiColor.g*ColorIngameAcaleA,
		ms_GuiColor.b*ColorIngameAcaleA,
		ms_GuiColor.a);

	// update the ui
	CUIRect *pScreen = UI()->Screen();
	float mx = (m_MousePos.x/(float)Graphics()->ScreenWidth())*pScreen->w;
	float my = (m_MousePos.y/(float)Graphics()->ScreenHeight())*pScreen->h;

	int Buttons = 0;
	if(m_UseMouseButtons)
	{
		if(Input()->KeyPressed(KEY_MOUSE_1)) Buttons |= 1;
		if(Input()->KeyPressed(KEY_MOUSE_2)) Buttons |= 2;
		if(Input()->KeyPressed(KEY_MOUSE_3)) Buttons |= 4;
	}

	UI()->Update(mx,my,mx*3.0f,my*3.0f,Buttons);

	// render
	if(Client()->State() != IClient::STATE_DEMOPLAYBACK && IsActive())
		Render();

	if(m_QAActive)
		RenderQA(*UI()->Screen());

	// render cursor
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,1);
	IGraphics::CQuadItem QuadItem(mx, my, 24, 24);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();

	// render debug information
	if(g_Config.m_Debug)
	{
		CUIRect Screen = *UI()->Screen();
		Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "%p %p %p", UI()->HotItem(), UI()->ActiveItem(), UI()->LastActiveItem());
		CTextCursor Cursor;
		TextRender()->SetCursor(&Cursor, 10, 10, 10, TEXTFLAG_RENDER);
		TextRender()->TextEx(&Cursor, aBuf, -1);
	}

	m_EscapePressed = false;
	m_EnterPressed = false;
	m_DeletePressed = false;
	m_NumInputEvents = 0;
}

void CMenus::RenderBackground()
{
	if (gs_TextureBlob == -1)
		gs_TextureBlob = Graphics()->LoadTexture("blob.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);

	if (Client()->State() == IClient::STATE_OFFLINE && m_UseShellmap)
	{
		if (!Client()->Map()->IsLoaded())
		{
			if (!Client()->Map()->Load("xclient/shell.map"))
			{
				dbg_msg("background", "shellmap not found");
				m_UseShellmap = false;
			}

			m_pClient->Layers()->Init(Kernel());
			RenderTools()->RenderTilemapGenerateSkip(Layers());
			for (int i = 0; i < m_pClient->All().m_Num; i++)
			{
				m_pClient->All().m_paComponents[i]->OnMapLoad();
				m_pClient->All().m_paComponents[i]->OnReset();
			}
		}
		else
		{
			CCamera *pCam = m_pClient->m_pCamera;
			vec2 Pos = vec2(0, 0);
			float z = -1.0f;

			m_pClient->m_pMapLayersBackGround->EnvelopeUpdate();
			m_pClient->m_pMapLayersForeGround->EnvelopeUpdate();

			// default is center of map
			Pos = m_pClient->m_MapSize*0.5f;
			z = 2.0f;

			if (m_Popup == POPUP_QUIT)
			{
				Pos = vec2(222.0f, 130.0f);
				z = 0.80f;
			}
			else if (m_ActivePage == PAGE_NEWS)
			{
				Pos = vec2(20.0f, 95.0f);
				z = 0.5f;
			}
			else if (m_ActivePage == PAGE_BROWSER)
			{
				Pos = vec2(60.0f, 160.0f);
				z = 0.75f;
			}
			else if (m_ActivePage == PAGE_SETTINGS)
			{
				Pos = vec2(135.0f, 178.0f);
				z = 0.5f;
			}
			if (z != -1.0f && Pos != vec2(0, 0))
			{
				vec2 MouseRelation = m_MousePos/vec2((float)Graphics()->ScreenWidth(), (float)Graphics()->ScreenHeight());
				pCam->Move(Pos-MouseRelation*2, z);
			}

			/*const CSkins::CSkin *pOwnSkin = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(g_Config.m_PlayerSkin));
			if (pOwnSkin)
			{
				CTeeRenderInfo OwnSkinInfo;
				if (g_Config.m_PlayerUseCustomColor)
				{
					OwnSkinInfo.m_Texture = pOwnSkin->m_ColorTexture;
					OwnSkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorBody);
					OwnSkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColorV4(g_Config.m_PlayerColorFeet);
				}
				else
				{
					OwnSkinInfo.m_Texture = pOwnSkin->m_OrgTexture;
					OwnSkinInfo.m_ColorBody = vec4(1.0f, 1.0f, 1.0f, 1.0f);
					OwnSkinInfo.m_ColorFeet = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				OwnSkinInfo.m_Size = 64.0f*z;

				RenderTools()->RenderTee(CAnimState::GetIdle(), &OwnSkinInfo, 0, vec2(1, 0), vec2(30.0f, 28.0f));
			}*/
		}
	}
	else
	{
		float sw = 300 * Graphics()->ScreenAspect();
		float sh = 300;
		Graphics()->MapScreen(0, 0, sw, sh);

		// render background color
		Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();
		vec4 Bottom(ms_GuiColor.r, ms_GuiColor.g, ms_GuiColor.b, 1.0f);
		vec4 Top(ms_GuiColor.r, ms_GuiColor.g, ms_GuiColor.b, 1.0f);
		IGraphics::CColorVertex Array[4] = {
			IGraphics::CColorVertex(0, Top.r, Top.g, Top.b, Top.a),
			IGraphics::CColorVertex(1, Top.r, Top.g, Top.b, Top.a),
			IGraphics::CColorVertex(2, Bottom.r, Bottom.g, Bottom.b, Bottom.a),
			IGraphics::CColorVertex(3, Bottom.r, Bottom.g, Bottom.b, Bottom.a) };
		Graphics()->SetColorVertex(Array, 4);
		IGraphics::CQuadItem QuadItem(0, 0, sw, sh);
		Graphics()->QuadsDrawTL(&QuadItem, 1);
		Graphics()->QuadsEnd();

		// render the tiles
		Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();
		float Size = 15.0f;
		float OffsetTime = fmod(Client()->LocalTime()*0.15f, 2.0f);
		for (int y = -2; y < (int)(sw / Size); y++)
			for (int x = -2; x < (int)(sh / Size); x++)
			{
			Graphics()->SetColor(0, 0, 0, 0.045f);
			IGraphics::CQuadItem QuadItem((x - OffsetTime)*Size * 2 + (y & 1)*Size, (y + OffsetTime)*Size, Size, Size);
			Graphics()->QuadsDrawTL(&QuadItem, 1);
			}
		Graphics()->QuadsEnd();

		// render border fade
		Graphics()->TextureSet(gs_TextureBlob);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(0, 0, 0, 0.5f);
		QuadItem = IGraphics::CQuadItem(-100, -100, sw + 200, sh + 200);
		Graphics()->QuadsDrawTL(&QuadItem, 1);
		Graphics()->QuadsEnd();

		// restore screen
		{CUIRect Screen = *UI()->Screen();
		Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h); }
	}
}

//void CMenus::RenderBackground()
//{
//	//Graphics()->Clear(0.25f,0.25f,0.25f);
//	if(gs_TextureBlob == -1)
//		gs_TextureBlob = Graphics()->LoadTexture("bg.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
//
//	float sw = UI()->Screen()->w;
//	float sh = UI()->Screen()->h;
//	//Graphics()->MapScreen(0, 0, sw, sh);
//
//	Graphics()->TextureSet(gs_TextureBlob);
//	Graphics()->QuadsBegin();
//		float Size = 200.0f;
//		for(int y = 0; y <= (int)(sw/Size)+2; y++)
//			for(int x = 0; x <= (int)(sh/Size)+2; x++)
//			{
//				IGraphics::CQuadItem QuadItem(x*Size, y*Size, Size, Size);
//				Graphics()->QuadsDrawTL(&QuadItem, 1);
//			}
//	Graphics()->QuadsEnd();
//
//	// restore screen
//	{CUIRect Screen = *UI()->Screen();
//	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);}
//
//
//	float var[4];
//		
//	Graphics()->ShaderSet(IGraphics::SHADER_HEXAGON);
//	Graphics()->ShaderBegin();
//
//		var[0] = 1.0f;
//		Graphics()->ShaderUniformSet("u_IsTex", var, 1);
//		var[0] = UI()->Screen()->w*2.0f;
//		var[1] = UI()->Screen()->h*2.0f;
//		Graphics()->ShaderUniformSet("u_Resolution", var, 2);
//		var[0] = Client()->LocalTime();
//		Graphics()->ShaderUniformSet("u_Time", var, 1);
//				
//		Graphics()->TextureSet(-1);
//		Graphics()->QuadsBegin();
//			IGraphics::CQuadItem QuadItem(0, 0, UI()->Screen()->w, UI()->Screen()->h);
//			Graphics()->QuadsDrawTL(&QuadItem, 1);
//		Graphics()->QuadsEnd();
//	Graphics()->ShaderEnd();
//	
//}

//void CMenus::RenderBackground()
//{
//	//Graphics()->Clear(1,1,1);
//	//render_sunrays(0,0);
//	if(gs_TextureBlob == -1)
//		gs_TextureBlob = Graphics()->LoadTexture("blob.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
//
//
//	float sw = 300*Graphics()->ScreenAspect();
//	float sh = 300;
//	Graphics()->MapScreen(0, 0, sw, sh);
//
//	// render background color
//	Graphics()->TextureSet(-1);
//	Graphics()->QuadsBegin();
//		//vec4 bottom(gui_color.r*0.3f, gui_color.g*0.3f, gui_color.b*0.3f, 1.0f);
//		//vec4 bottom(0, 0, 0, 1.0f);
//		vec4 Bottom(ms_GuiColor.r, ms_GuiColor.g, ms_GuiColor.b, 1.0f);
//		vec4 Top(ms_GuiColor.r, ms_GuiColor.g, ms_GuiColor.b, 1.0f);
//		IGraphics::CColorVertex Array[4] = {
//			IGraphics::CColorVertex(0, Top.r, Top.g, Top.b, Top.a),
//			IGraphics::CColorVertex(1, Top.r, Top.g, Top.b, Top.a),
//			IGraphics::CColorVertex(2, Bottom.r, Bottom.g, Bottom.b, Bottom.a),
//			IGraphics::CColorVertex(3, Bottom.r, Bottom.g, Bottom.b, Bottom.a)};
//		Graphics()->SetColorVertex(Array, 4);
//		IGraphics::CQuadItem QuadItem(0, 0, sw, sh);
//		Graphics()->QuadsDrawTL(&QuadItem, 1);
//	Graphics()->QuadsEnd();
//
//	// render the tiles
//	Graphics()->TextureSet(-1);
//	Graphics()->QuadsBegin();
//		float Size = 15.0f;
//		float OffsetTime = fmod(Client()->LocalTime()*0.15f, 2.0f);
//		for(int y = -2; y < (int)(sw/Size); y++)
//			for(int x = -2; x < (int)(sh/Size); x++)
//			{
//				Graphics()->SetColor(0,0,0,0.045f);
//				IGraphics::CQuadItem QuadItem((x-OffsetTime)*Size*2+(y&1)*Size, (y+OffsetTime)*Size, Size, Size);
//				Graphics()->QuadsDrawTL(&QuadItem, 1);
//			}
//	Graphics()->QuadsEnd();
//
//	// render border fade
//	Graphics()->TextureSet(gs_TextureBlob);
//	Graphics()->QuadsBegin();
//		Graphics()->SetColor(0,0,0,0.5f);
//		QuadItem = IGraphics::CQuadItem(-100, -100, sw+200, sh+200);
//		Graphics()->QuadsDrawTL(&QuadItem, 1);
//	Graphics()->QuadsEnd();
//
//	// restore screen
//	{CUIRect Screen = *UI()->Screen();
//	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);}
//}
