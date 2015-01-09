#include <windows.h>
#include <base/system.h>
#include <engine/shared/protocol.h>

#include "libhandler.h"

void CLibHandler::Init()
{
	for(int i = 0; i < NUM_LIBS; i++)
		CallFunction[i] = NULL;

	LoadLib(LIB_USER, "cache/funclib_06u.dll");
	if(1 == 0) //TODO
	{
		LoadLib(LIB_VIP, "cache/funclib_06v.dll");
		LoadLib(LIB_ADMIN, "cache/funclib_06a.dll");
	}
}

void CLibHandler::LoadLib(int Lib, const char *pLib)
{
	HMODULE hLib = NULL;

	hLib = LoadLibrary(pLib);
	if(hLib == NULL)
	{
		dbg_msg("13x37", "failed to load %s", pLib);
		return;
	}

	CallFunction[Lib] = (FUNC_CALLFUNCTION)GetProcAddress(hLib, "CallFunction");
	if(CallFunction[Lib] == NULL) 
	{
		dbg_msg("13x37", "failed to load function");
		return;
	}

	if(!CallFunction[Lib](CALL_INIT, 0))
	{	
		dbg_msg("13x37", "failed to init %s", pLib);
		return;
	}

	m_pHandle[Lib] = (void*)hLib;
}

void CLibHandler::Shutdown()
{
	for(int i = 0; i < NUM_LIBS; i++)
		FreeLibrary((HMODULE)m_pHandle[i]); 
}
