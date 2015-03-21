/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_TEEFILES_H
#define GAME_CLIENT_COMPONENTS_TEEFILES_H
#include <base/vmath.h>
#include <base/tl/sorted_array.h>
#include <engine/shared/linereader.h>
#include <game/client/component.h>

class CTeeFiles : public CComponent
{
public:
	// do this better and nicer
	struct CTee
	{
		char m_aFilename[41];
		char m_aName[16];
		char m_aClan[12];
		char m_aSkin[24];
		int m_UseCustomColor;
		int m_ColorBody;
		int m_ColorFeet;
		int m_Country;

		bool operator<(const CTee &Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};

	enum
	{
		TEE_NAME=0,
		TEE_CLAN,
		TEE_SKIN,
		TEE_USECUSTOMCOLOR,
		TEE_COLORBODY,
		TEE_COLORFEET,
		TEE_COUNTRY,
		NUM_TEE_ENTRIES,
	};

	void OnInit();

	CTee *Get(int Index);
	void Save();
	void Add(CTee Tee);
	void Remove(int Id);
	int Num();
	int Find(const char *pName);


private:
	sorted_array<CTee> m_aTees;

	static int TeeFileScan(const char *pName, int IsDir, int DirType, void *pUser);
};
#endif
