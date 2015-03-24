/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>

#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/shared/config.h>
#include <engine/shared/md5.h>

#include "teefiles.h"

int CTeeFiles::TeeFileScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CTeeFiles *pSelf = (CTeeFiles*)pUser;
	int l = str_length(pName);
	if(l < 4 || IsDir || str_comp(pName+l-4, ".tee") != 0)
		return 0;

	char aBuf[512];
	char aFileName[41];
	char aFilePath[512];
	char aTeeEntry[NUM_TEE_ENTRIES][32];
	str_format(aFilePath, sizeof(aFilePath), "xclient/teefiles/%s", pName);
	str_format(aFileName, sizeof(aFileName), pName);

	IOHANDLE File = pSelf->Storage()->OpenFile(aFilePath, IOFLAG_READ, IStorage::TYPE_SAVE);

	if(File)
	{
		char *pLine;
		CLineReader lr;

		lr.Init(File);

		for(int i = 0; i < NUM_TEE_ENTRIES; i++)
		{
			if(!(pLine = lr.Get()))
			{
				str_format(aBuf, sizeof(aBuf), "failed to read '%s' at line %d", aFileName, i);
				pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "console", aBuf);
				io_close(File);
				mem_zero(aTeeEntry[i], sizeof(aTeeEntry[i]));
				break;
			}
			str_format(aTeeEntry[i], sizeof(aTeeEntry[i]), pLine);
		}

		io_close(File);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "failed to open '%s'", aFileName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "console", aBuf);
		return 1;
	}

	CTee Tee;
	str_format(Tee.m_aFilename, sizeof(Tee.m_aFilename), aFileName);
	str_format(Tee.m_aName, sizeof(Tee.m_aName), aTeeEntry[TEE_NAME]);
	str_format(Tee.m_aClan, sizeof(Tee.m_aClan), aTeeEntry[TEE_CLAN]);
	str_format(Tee.m_aSkin, sizeof(Tee.m_aSkin), aTeeEntry[TEE_SKIN]);
	Tee.m_UseCustomColor = str_toint(aTeeEntry[TEE_USECUSTOMCOLOR]);
	Tee.m_ColorBody = str_toint(aTeeEntry[TEE_COLORBODY]);
	Tee.m_ColorFeet = str_toint(aTeeEntry[TEE_COLORFEET]);
	Tee.m_Country = str_toint(aTeeEntry[TEE_COUNTRY]);
	pSelf->m_aTees.add(Tee);

	return 0;
}

void CTeeFiles::Save()
{
	const int sz = 32;
	for(int i = 0; i < m_aTees.size(); i++)
	{
		CTee *pTee = Get(i);
		bool Rename = false;
		char aFilename[41]; // + ".tee" + "xxx_"
		char aBuf[512];

		if(pTee->m_aFilename[0] == '\0')
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "teefile%d%s%s%s%d%d%d%d", i, pTee->m_aName, pTee->m_aClan, pTee->m_aSkin, pTee->m_UseCustomColor, pTee->m_ColorBody, pTee->m_ColorFeet, pTee->m_Country);
			str_format(aFilename, sizeof(aFilename), "%03d_%s.tee", i, md5(aBuf));
		}
		else
		{
			if(!(str_isdigit(pTee->m_aFilename[0]) && str_isdigit(pTee->m_aFilename[1]) && str_isdigit(pTee->m_aFilename[2]) && pTee->m_aFilename[3] == '_'))
			{
				str_format(aFilename, sizeof(aFilename), "%03d_%s", i, pTee->m_aFilename);
				Rename = true;
			}
			else
			{ 
				char aNewIndex[4];
				char aOldIndex[4];
				str_format(aNewIndex, sizeof(aNewIndex), "%03d", i);
				str_format(aOldIndex, sizeof(aOldIndex), pTee->m_aFilename);
				if(str_toint(aNewIndex) != str_toint(aOldIndex))
				{
					str_format(aFilename, sizeof(aFilename), pTee->m_aFilename);
					for(int i = 0; i < 3; i++)
						aFilename[i] = aNewIndex[i];
					Rename = true;
				}
			}
		}

		if(Rename)
		{
			char OldName[512];
			char NewName[512];
			str_format(OldName, sizeof(OldName), "xclient/teefiles/%s", pTee->m_aFilename);
			str_format(NewName, sizeof(NewName), "xclient/teefiles/%s", aFilename);
			Storage()->RenameFile(OldName, NewName, IStorage::TYPE_SAVE);
			Storage()->RemoveFile(OldName, IStorage::TYPE_SAVE);

			str_format(aBuf, sizeof(aBuf), "renamed '%s' to %s", pTee->m_aFilename, aFilename);
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "console", aBuf);
		}

		str_format(aBuf, sizeof(aBuf), "xclient/teefiles/%s", aFilename);
		IOHANDLE File = Storage()->OpenFile(aBuf, IOFLAG_WRITE, IStorage::TYPE_SAVE);
		
		if(!File)
			continue;

		char aTeeEntry[NUM_TEE_ENTRIES][sz];
		str_format(aTeeEntry[TEE_NAME], sz, pTee->m_aName);
		str_format(aTeeEntry[TEE_CLAN], sz, pTee->m_aClan);
		str_format(aTeeEntry[TEE_SKIN], sz, pTee->m_aSkin);
		str_format(aTeeEntry[TEE_USECUSTOMCOLOR], sz, "%d", pTee->m_UseCustomColor);
		str_format(aTeeEntry[TEE_COLORBODY], sz, "%d", pTee->m_ColorBody);
		str_format(aTeeEntry[TEE_COLORFEET], sz, "%d", pTee->m_ColorFeet);
		str_format(aTeeEntry[TEE_COUNTRY], sz, "%d", pTee->m_Country);
		
		for(int i = 0; i < NUM_TEE_ENTRIES; i++)
		{
			if(!File)
			{
				str_format(aBuf, sizeof(aBuf), "failed to save '%s' at line %d", aFilename, i);
				Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "console", aBuf);
				io_close(File);
				mem_zero(aTeeEntry[i], sizeof(aTeeEntry[i]));
				break;
			}
			io_write(File, aTeeEntry[i], str_length(aTeeEntry[i]));
			io_write_newline(File);
		}
		io_close(File);
	}	
}

void CTeeFiles::Add(CTee Tee)
{
	m_aTees.add(Tee);
}

void CTeeFiles::Remove(int Id)
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "xclient/teefiles/%s", m_aTees[Id].m_aFilename);
	Storage()->RemoveFile(aBuf, IStorage::TYPE_SAVE);
	m_aTees.remove_index(Id);
}

void CTeeFiles::OnInit()
{
	m_aTees.clear();
	Storage()->ListDirectory(IStorage::TYPE_SAVE, "xclient/teefiles", TeeFileScan, this);
	if(!m_aTees.size())
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "failed to load teefiles. folder='xclient/teefiles'");
	}
}

int CTeeFiles::Num()
{
	return m_aTees.size();
}

CTeeFiles::CTee *CTeeFiles::Get(int Index)
{
	if (m_aTees.size())
		return &m_aTees[max(0, Index%m_aTees.size())];
	else
		return NULL;
}

int CTeeFiles::Find(const char *pName)
{
	for(int i = 0; i < m_aTees.size(); i++)
	{
		if(str_comp(m_aTees[i].m_aFilename, pName) == 0)
			return i;
	}
	return -1;
}