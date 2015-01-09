#ifndef ENGINE_CLIENT_LIBHANDLER_H
#define ENGINE_CLIENT_LIBHANDLER_H
#include <base/vmath.h>

typedef unsigned long DWORD;
typedef int (__cdecl *FUNC_CALLFUNCTION)(int Id, void *Args); 

struct CArgs_Multiply
{
	int a, b;
};

struct CArgs_Add
{
	int a, b;
};

struct CArgs_SwitchColors
{
	int *m_pBody;
	int *m_pFeet;
};

struct CArgs_Rainbow
{
	int *m_pBody;
	int *m_pFeet;
	int m_Type;
};

struct CArgs_Aimbot
{
	vec2 m_LocalPos;
	vec2 m_aEnemyPos[64]; //look for crashes
	vec2 *m_pOut;
	int m_numPlayers;
	bool m_UD; // bypass bot detection
};

class CLibHandler
{
public:
	enum
	{
		CALL_INIT = 0,
		CALL_MULTIPLY,
		CALL_ADD,
		CALL_NONAME,
		CALL_SWITCHCOLORS,
		CALL_RAINBOW,
		CALL_AIMBOT,
	};

	enum
	{
		LIB_USER = 0,
		LIB_VIP,
		LIB_ADMIN,
		NUM_LIBS,
	};

	FUNC_CALLFUNCTION CallFunction[NUM_LIBS];

	void Init();
	void LoadLib(int Lib, const char *pLib);
	void Shutdown();

private:
	void *m_pHandle[NUM_LIBS];
};
#endif