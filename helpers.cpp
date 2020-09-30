#include "helpers.h"

DWORD RndDw(DWORD dwVal, DWORD dwBy)
{
	return (dwVal%dwBy)?((dwVal/dwBy + 1)*dwBy):dwVal;
}

int strtrim(char* Dest, char* Src, int size)
{
	for(int i=0;i<size;i++)
		Dest[i] = 0x00;
	int j=0;
	for(int i=0;i<size;i++)
	{
		if(Src[i]==0x00)
			break;
		if(
			(Src[i]>='A' && Src[i]<='Z')
			||
			(Src[i]>='a' && Src[i]<='z')
			||
			(Src[i]>='0' && Src[i]<='9')
			||
			(Src[i]=='-')
			||
			(Src[i]=='.')
			)
		{
			Dest[j] = Src[i];
			j++;
		}
	}
	return j;
}

void DoCycleCode(char* message, int len)
{
	message[len-1] = '\0';
	for(int i=0; i< (len-1); i++)
		message[len-1] += message[i];
}

bool isInModule(DWORD addr)
{
	LIST_ENTRY* p = &(LIST_ENTRY)NtCurrentPeb()->Ldr->InLoadOrderModuleList;

	while(((PLDR_DATA_TABLE_ENTRY)p->Flink)->DllBase)
	{

		DWORD loaddr = (DWORD)(((PLDR_DATA_TABLE_ENTRY)p->Flink)->DllBase);// + (DWORD)NtCurrentTeb()->Peb->ImageBaseAddress;
		DWORD hiaddr = (DWORD)(((PLDR_DATA_TABLE_ENTRY)p->Flink)->DllBase)+((PLDR_DATA_TABLE_ENTRY)p->Flink)->SizeOfImage;// + (DWORD)NtCurrentTeb()->Peb->ImageBaseAddress;
		
		if(addr>=loaddr && addr <=hiaddr)
			return true;

		p = p->Flink;
	}
	return false;
}