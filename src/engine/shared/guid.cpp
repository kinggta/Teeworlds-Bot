#include <base/system.h>
#if defined(CONF_FAMILY_WINDOWS)
	#include <windows.h>
#endif

#include "md5.h"
#include "guid.h"



void guid_get(char *buffer)
{
#if defined (CONF_FAMILY_WINDOWS)
	char SerialStr[200], Guid[64];
	DWORD SerialNum;
	GetVolumeInformation("c:\\", NULL, NULL, &SerialNum, NULL, NULL, NULL, NULL);
	str_format(SerialStr,sizeof(SerialStr), "%d", SerialNum );
	char tlas[64];
	str_format(tlas, sizeof(tlas), "%c%c%c%c%c%c%c%c%c%c%c%c",//eggswithsalt
		101, 103, 103, 115, 119, 105, 116, 104, 115, 97, 108, 116);
	strcat(SerialStr,tlas); 

	std::string TmpStr = md5(md5(md5(SerialStr))); 
	for(int i=0; i <= TmpStr.size(); i++) 
		Guid[i] = TmpStr[i];
	str_format(buffer,sizeof(Guid), Guid); 
#else
	str_format(buffer, sizeof("N/A"), "N/A");
#endif
}
	