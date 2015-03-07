/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_VARIABLES_H
#define GAME_VARIABLES_H
#undef GAME_VARIABLES_H // this file will be included several times


// client
MACRO_CONFIG_INT(ClPredict, cl_predict, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Predict client movements")
MACRO_CONFIG_INT(ClNameplates, cl_nameplates, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show name plates")
MACRO_CONFIG_INT(ClNameplatesAlways, cl_nameplates_always, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Always show name plates disregarding of distance")
MACRO_CONFIG_INT(ClNameplatesTeamcolors, cl_nameplates_teamcolors, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Use team colors for name plates")
MACRO_CONFIG_INT(ClNameplatesSize, cl_nameplates_size, 50, 0, 100, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Size of the name plates from 0 to 100%")
MACRO_CONFIG_INT(ClAutoswitchWeapons, cl_autoswitch_weapons, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Auto switch weapon on pickup")

MACRO_CONFIG_INT(ClShowhud, cl_showhud, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show ingame HUD")
MACRO_CONFIG_INT(ClShowChatFriends, cl_show_chat_friends, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show only chat messages from friends")
MACRO_CONFIG_INT(ClShowfps, cl_showfps, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show ingame FPS counter")

MACRO_CONFIG_INT(ClAirjumpindicator, cl_airjumpindicator, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClThreadsoundloading, cl_threadsoundloading, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Load sound files threaded")

MACRO_CONFIG_INT(ClWarningTeambalance, cl_warning_teambalance, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Warn about team balance")

MACRO_CONFIG_INT(ClMouseDeadzone, cl_mouse_deadzone, 300, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClMouseFollowfactor, cl_mouse_followfactor, 60, 0, 200, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClMouseMaxDistance, cl_mouse_max_distance, 800, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(EdShowkeys, ed_showkeys, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

//MACRO_CONFIG_INT(ClFlow, cl_flow, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(ClShowWelcome, cl_show_welcome, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClMotdTime, cl_motd_time, 10, 0, 100, CFGFLAG_CLIENT|CFGFLAG_SAVE, "How long to show the server message of the day")

MACRO_CONFIG_STR(ClVersionServer, cl_version_server, 100, "version.teeworlds.com", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Server to use to check for new versions")

MACRO_CONFIG_STR(ClLanguagefile, cl_languagefile, 255, "", CFGFLAG_CLIENT|CFGFLAG_SAVE, "What language file to use")

MACRO_CONFIG_INT(PlayerUseCustomColor, player_use_custom_color, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(PlayerColorBody, player_color_body, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player body color")
MACRO_CONFIG_INT(PlayerColorFeet, player_color_feet, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player feet color")
MACRO_CONFIG_STR(PlayerSkin, player_skin, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player skin")

MACRO_CONFIG_INT(UiPage, ui_page, 6, 0, 10, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface page")
MACRO_CONFIG_INT(UiToolboxPage, ui_toolbox_page, 0, 0, 2, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toolbox page")
MACRO_CONFIG_STR(UiServerAddress, ui_server_address, 64, "localhost:8303", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface server address")
MACRO_CONFIG_INT(UiScale, ui_scale, 100, 50, 150, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface scale")
MACRO_CONFIG_INT(UiMousesens, ui_mousesens, 100, 5, 100000, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Mouse sensitivity for menus/editor")

MACRO_CONFIG_INT(UiColorHue, ui_color_hue, 160, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color hue")
MACRO_CONFIG_INT(UiColorSat, ui_color_sat, 70, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color saturation")
MACRO_CONFIG_INT(UiColorLht, ui_color_lht, 175, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color lightness")
MACRO_CONFIG_INT(UiColorAlpha, ui_color_alpha, 228, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface alpha")

MACRO_CONFIG_INT(GfxNoclip, gfx_noclip, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Disable clipping")

MACRO_CONFIG_INT(ClShader, cl_shader, 0, 0, 9999, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

// SX

// Identities // DO NOT CHANGE ORDER
MACRO_CONFIG_INT(XFakeId, x_fake_id, 0, 0, 6, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake Id")
MACRO_CONFIG_STR(XFakeName1, x_fake_name1, 16, "fakeless tee", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Name of the fake")
MACRO_CONFIG_STR(XFakeClan1, x_fake_clan1, 12, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Clan of the fake")
MACRO_CONFIG_INT(XFakeUseCustomColor1, x_fake_use_custom_color1, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(XFakeColorBody1, x_fake_color_body1, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake body color")
MACRO_CONFIG_INT(XFakeColorFeet1, x_fake_color_feet1, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake feet color")
MACRO_CONFIG_STR(XFakeSkin1, x_fake_skin1, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake skin")
MACRO_CONFIG_STR(XFakeName2, x_fake_name2, 16, "fakeless tee", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Name of the fake")
MACRO_CONFIG_STR(XFakeClan2, x_fake_clan2, 12, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Clan of the fake")
MACRO_CONFIG_INT(XFakeUseCustomColor2, x_fake_use_custom_color2, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(XFakeColorBody2, x_fake_color_body2, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake body color")
MACRO_CONFIG_INT(XFakeColorFeet2, x_fake_color_feet2, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake feet color")
MACRO_CONFIG_STR(XFakeSkin2, x_fake_skin2, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake skin")
MACRO_CONFIG_STR(XFakeName3, x_fake_name3, 16, "fakeless tee", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Name of the fake")
MACRO_CONFIG_STR(XFakeClan3, x_fake_clan3, 12, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Clan of the fake")
MACRO_CONFIG_INT(XFakeUseCustomColor3, x_fake_use_custom_color3, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(XFakeColorBody3, x_fake_color_body3, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake body color")
MACRO_CONFIG_INT(XFakeColorFeet3, x_fake_color_feet3, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake feet color")
MACRO_CONFIG_STR(XFakeSkin3, x_fake_skin3, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake skin")
MACRO_CONFIG_STR(XFakeName4, x_fake_name4, 16, "fakeless tee", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Name of the fake")
MACRO_CONFIG_STR(XFakeClan4, x_fake_clan4, 12, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Clan of the fake")
MACRO_CONFIG_INT(XFakeUseCustomColor4, x_fake_use_custom_color4, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(XFakeColorBody4, x_fake_color_body4, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake body color")
MACRO_CONFIG_INT(XFakeColorFeet4, x_fake_color_feet4, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake feet color")
MACRO_CONFIG_STR(XFakeSkin4, x_fake_skin4, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake skin")
MACRO_CONFIG_STR(XFakeName5, x_fake_name5, 16, "fakeless tee", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Name of the fake")
MACRO_CONFIG_STR(XFakeClan5, x_fake_clan5, 12, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Clan of the fake")
MACRO_CONFIG_INT(XFakeUseCustomColor5, x_fake_use_custom_color5, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(XFakeColorBody5, x_fake_color_body5, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake body color")
MACRO_CONFIG_INT(XFakeColorFeet5, x_fake_color_feet5, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake feet color")
MACRO_CONFIG_STR(XFakeSkin5, x_fake_skin5, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Fake skin")

// Aimbot
MACRO_CONFIG_INT(XAimbot, x_aimbot, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "I think you know what it does")
MACRO_CONFIG_INT(XAimbotBypass, x_aimbot_bypass, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Bypass simple bot detections")
MACRO_CONFIG_INT(XAimbotEnemycheck, x_aimbot_enemycheck, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Only aim at enemies")
MACRO_CONFIG_INT(XAimbotFriendcheck, x_aimbot_friendcheck, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Do not aim at friends")
MACRO_CONFIG_INT(XAimbotWallcheck, x_aimbot_wallcheck, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Do not target enemies behind walls when turned on")
MACRO_CONFIG_INT(XAimbotTeamcheck, x_aimbot_teamcheck, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Do not target enemies from the same team when turned on")
MACRO_CONFIG_INT(XAimbotRange, x_aimbot_range, 700, 0, 9999, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Do not target enemies behind the range")
MACRO_CONFIG_INT(XAimbotRangeFromTuning, x_aimbot_range_from_tunings, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "copy the laser reach from the servers tuning")
MACRO_CONFIG_INT(XAimbotHotkey, x_aimbot_hotkey, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Use hotkey to aim (set with 'bind <key> +aim')")
MACRO_CONFIG_INT(XAimbotAutoshot, x_aimbot_autoshot, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Shoot on sight")
MACRO_CONFIG_INT(XAimbotAutohook, x_aimbot_autohook, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Hook on sight")

MACRO_CONFIG_INT(XTriggerbot, x_triggerbot, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Shoot when target is in line of aim")


// Spinbot
MACRO_CONFIG_INT(XSpinbot, x_spinbot, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "You spin my head right round..")
MACRO_CONFIG_INT(XSpinbotDirection, x_spinbot_direction, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Direction of spin")
MACRO_CONFIG_INT(XSpinbotSpeed, x_spinbot_speed, 50, 1, 100, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Speed of spin")

// Chatbot
MACRO_CONFIG_INT(XChatbotRepeat, x_chatbot_repeat, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Be a parrot")
MACRO_CONFIG_INT(XChatbotFlame, x_chatbot_flame, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Fuck fairplay")
MACRO_CONFIG_INT(XChatbotPraise, x_chatbot_praise, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Do not fuck fairplay")
MACRO_CONFIG_INT(XChatbotPingAll, x_chatbot_ping_all, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Red chat for everyone")

// Movebot
MACRO_CONFIG_INT(XMovebotFly, x_movebot_fly, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Fly with your hook")
MACRO_CONFIG_INT(XMovebotBalance, x_movebot_balance, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Balance on other tees")
MACRO_CONFIG_INT(XMovebotRocketjump, x_movebot_rocketjump, 0, 0, 3, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Rocketjump with x rockets")

// Dummies
MACRO_CONFIG_STR(XDummyName1, x_dummy_name1, 16, "dummy", CFGFLAG_SAVE | CFGFLAG_CLIENT, "Name of the dummy")
MACRO_CONFIG_STR(XDummyClan1, x_dummy_clan1, 12, "", CFGFLAG_SAVE | CFGFLAG_CLIENT, "Clan of the fake")
MACRO_CONFIG_INT(XDummyUseCustomColor1, x_dummy_use_custom_color1, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(XDummyColorBody1, x_dummy_color_body1, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy body color")
MACRO_CONFIG_INT(XDummyColorFeet1, x_dummy_color_feet1, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy feet color")
MACRO_CONFIG_STR(XDummySkin1, x_dummy_skin1, 24, "default", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy skin")
MACRO_CONFIG_STR(XDummyName2, x_dummy_name2, 16, "dummy", CFGFLAG_SAVE | CFGFLAG_CLIENT, "Name of the dummy")
MACRO_CONFIG_STR(XDummyClan2, x_dummy_clan2, 12, "", CFGFLAG_SAVE | CFGFLAG_CLIENT, "Clan of the fake")
MACRO_CONFIG_INT(XDummyUseCustomColor2, x_dummy_use_custom_color2, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(XDummyColorBody2, x_dummy_color_body2, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy body color")
MACRO_CONFIG_INT(XDummyColorFeet2, x_dummy_color_feet2, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy feet color")
MACRO_CONFIG_STR(XDummySkin2, x_dummy_skin2, 24, "default", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy skin")
MACRO_CONFIG_STR(XDummyName3, x_dummy_name3, 16, "dummy", CFGFLAG_SAVE | CFGFLAG_CLIENT, "Name of the dummy")
MACRO_CONFIG_STR(XDummyClan3, x_dummy_clan3, 12, "", CFGFLAG_SAVE | CFGFLAG_CLIENT, "Clan of the fake")
MACRO_CONFIG_INT(XDummyUseCustomColor3, x_dummy_use_custom_color3, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(XDummyColorBody3, x_dummy_color_body3, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy body color")
MACRO_CONFIG_INT(XDummyColorFeet3, x_dummy_color_feet3, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy feet color")
MACRO_CONFIG_STR(XDummySkin3, x_dummy_skin3, 24, "default", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Dummy skin")

// Autoreconnect
MACRO_CONFIG_INT(XAutoReconnect, x_autoreconnect, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Automatically reconnect when server is full")
MACRO_CONFIG_INT(XAutoReconnectTime, x_autoreconnect_time, 5, 1, 10, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Frequency time to reconnect")

// Misc
MACRO_CONFIG_STR(XBindaddr, x_econ_addr, 128, "localhost", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Address to bind the external console to. Anything but 'localhost' is dangerous")
MACRO_CONFIG_INT(XShowOthers, x_show_others, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Snap players from other teams (DDRace)")
MACRO_CONFIG_INT(XFriendHighlight, x_friend_highlight, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Highlight friends ingame with a special skin and color")
MACRO_CONFIG_STR(XFriendSkin, x_friend_skin, 24, "default", CFGFLAG_CLIENT | CFGFLAG_SAVE, "skin for highlighted friends")
MACRO_CONFIG_INT(XEnemyHighlight, x_enemy_highlight, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Highlight enemies ingame with a special skin and color")
MACRO_CONFIG_STR(XEnemySkin, x_enemy_skin, 24, "default", CFGFLAG_CLIENT | CFGFLAG_SAVE, "skin for highlighted enemies")
MACRO_CONFIG_INT(XSoundHighlight, x_sound_highlight, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Play highlight sound even when minimized")
MACRO_CONFIG_INT(XSoundMuteChat, x_sound_mute_chat, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Mute chat sound")
MACRO_CONFIG_INT(XSoundMuteServer, x_sound_mute_server, 0, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Mute server chat sound")

// Rainbow
MACRO_CONFIG_INT(XRainbow, x_rainbow, 0, 0, 3, CFGFLAG_CLIENT, "Activate rainbow (1=Body, 2=Feet, 3=Both")
MACRO_CONFIG_INT(XRainbowSkin, x_rainbow_skin, 0, 0, 3, CFGFLAG_CLIENT, "Activate skin rainbow (1=Body, 2=Feet, 3=Both")


// Account
MACRO_CONFIG_STR(XLoginName, x_login_name, 32, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "")
MACRO_CONFIG_STR(XLoginPass, x_login_pass, 32, "", CFGFLAG_CLIENT, "")

// server
MACRO_CONFIG_INT(SvWarmup, sv_warmup, 0, 0, 0, CFGFLAG_SERVER, "Number of seconds to do warmup before round starts")
MACRO_CONFIG_STR(SvMotd, sv_motd, 900, "", CFGFLAG_SERVER, "Message of the day to display for the clients")
MACRO_CONFIG_INT(SvTeamdamage, sv_teamdamage, 0, 0, 1, CFGFLAG_SERVER, "Team damage")
MACRO_CONFIG_STR(SvMaprotation, sv_maprotation, 768, "", CFGFLAG_SERVER, "Maps to rotate between")
MACRO_CONFIG_INT(SvRoundsPerMap, sv_rounds_per_map, 1, 1, 100, CFGFLAG_SERVER, "Number of rounds on each map before rotating")
MACRO_CONFIG_INT(SvRoundSwap, sv_round_swap, 1, 0, 1, CFGFLAG_SERVER, "Swap teams between rounds")
MACRO_CONFIG_INT(SvPowerups, sv_powerups, 1, 0, 1, CFGFLAG_SERVER, "Allow powerups like ninja")
MACRO_CONFIG_INT(SvScorelimit, sv_scorelimit, 20, 0, 1000, CFGFLAG_SERVER, "Score limit (0 disables)")
MACRO_CONFIG_INT(SvTimelimit, sv_timelimit, 0, 0, 1000, CFGFLAG_SERVER, "Time limit in minutes (0 disables)")
MACRO_CONFIG_STR(SvGametype, sv_gametype, 32, "dm", CFGFLAG_SERVER, "Game type (dm, tdm, ctf)")
MACRO_CONFIG_INT(SvTournamentMode, sv_tournament_mode, 0, 0, 1, CFGFLAG_SERVER, "Tournament mode. When enabled, players joins the server as spectator")
MACRO_CONFIG_INT(SvSpamprotection, sv_spamprotection, 1, 0, 1, CFGFLAG_SERVER, "Spam protection")

MACRO_CONFIG_INT(SvRespawnDelayTDM, sv_respawn_delay_tdm, 3, 0, 10, CFGFLAG_SERVER, "Time needed to respawn after death in tdm gametype")

MACRO_CONFIG_INT(SvSpectatorSlots, sv_spectator_slots, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Number of slots to reserve for spectators")
MACRO_CONFIG_INT(SvTeambalanceTime, sv_teambalance_time, 1, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before autobalancing teams")
MACRO_CONFIG_INT(SvInactiveKickTime, sv_inactivekick_time, 3, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before taking care of inactive players")
MACRO_CONFIG_INT(SvInactiveKick, sv_inactivekick, 1, 0, 2, CFGFLAG_SERVER, "How to deal with inactive players (0=move to spectator, 1=move to free spectator slot/kick, 2=kick)")

MACRO_CONFIG_INT(SvStrictSpectateMode, sv_strict_spectate_mode, 0, 0, 1, CFGFLAG_SERVER, "Restricts information in spectator mode")
MACRO_CONFIG_INT(SvVoteSpectate, sv_vote_spectate, 1, 0, 1, CFGFLAG_SERVER, "Allow voting to move players to spectators")
MACRO_CONFIG_INT(SvVoteSpectateRejoindelay, sv_vote_spectate_rejoindelay, 3, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before a player can rejoin after being moved to spectators by vote")
MACRO_CONFIG_INT(SvVoteKick, sv_vote_kick, 1, 0, 1, CFGFLAG_SERVER, "Allow voting to kick players")
MACRO_CONFIG_INT(SvVoteKickMin, sv_vote_kick_min, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Minimum number of players required to start a kick vote")
MACRO_CONFIG_INT(SvVoteKickBantime, sv_vote_kick_bantime, 5, 0, 1440, CFGFLAG_SERVER, "The time to ban a player if kicked by vote. 0 makes it just use kick")

// debug
#ifdef CONF_DEBUG // this one can crash the server if not used correctly
	MACRO_CONFIG_INT(DbgDummies, dbg_dummies, 0, 0, 15, CFGFLAG_SERVER, "")
#endif

MACRO_CONFIG_INT(DbgFocus, dbg_focus, 0, 0, 1, CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(DbgTuning, dbg_tuning, 0, 0, 1, CFGFLAG_CLIENT, "")
#endif
