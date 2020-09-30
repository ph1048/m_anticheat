#include "hwid.h"
#include "strcrypt.h"

void GetHWID(char* hwid)
{
	//VMProtectBeginUltra("hwidgen");
	char szCpuInfo[64] = {0};
	char szHddInfo[64] = {0};
	char szTmp[128] = {0};
	getIDs(szHddInfo);
	_p_info info = {0};
	_cpuid(&info);
	__try
	{
		wsprintf(szCpuInfo,F("%s %s %d %d"),info.v_name,info.model_name,info.stepping,info.family);
	}
	__except(1)
	{
		wsprintf(szCpuInfo,F("%d %d %d %d"),info.feature,info.model,info.stepping,info.family);
	}
	strcpy(szTmp,szHddInfo);
	strcat(szTmp,szCpuInfo);
	char* md5 = CMD5Checksum::GetMD5((BYTE*)szTmp,(UINT)strnlen_s(szTmp,128));
	strcpy(hwid,md5);
	//VMProtectEnd();
}