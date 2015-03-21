#include <base/system.h>
#if defined(CONF_FAMILY_WINDOWS)
	#include <windows.h>
#endif

#include "md5.h"
#include "guid.h"

void guid_get(char *buffer)
{
	char aGuid[32];

#if defined (CONF_FAMILY_WINDOWS)
	char aSerial[200];
	DWORD SerialNum;
	GetVolumeInformation("c:\\", NULL, NULL, &SerialNum, NULL, NULL, NULL, NULL);
	str_format(aSerial, sizeof(aSerial), "%d", SerialNum);
	char Salt[64];
	str_format(Salt, sizeof(Salt), "%c%c%c%c%c%c%c%c%c%c%c%c",//eggswithsalt
		101, 103, 103, 115, 119, 105, 116, 104, 115, 97, 108, 116);
	strcat(aSerial, Salt);
	str_format(aGuid, sizeof(aGuid), "%s", md5(md5(md5(aSerial))));
#else
	str_format(aGuid, sizeof("N/A"), "N/A");
#endif

	str_format(buffer, sizeof(aGuid), aGuid);
}
	