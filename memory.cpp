#include "memory.h"
#include "strcrypt.h"

struct ControlPair
{
	HMODULE hModule;
	DWORD dwChecksum;
};

struct ControlPairs
{
	int count;
	ControlPair pair[16];
};

ControlPairs cp = {0};

int ProtectModule(DWORD dwModuleAddress)
{
#ifdef ANTIDBG
	BYTE* bPEHdr = NULL;
	DWORD dwSizeOfHeader = 0x1000;
	if(STATUS_SUCCESS != NtAllocateVirtualMemory(NtCurrentProcess(),(PVOID*)&bPEHdr,NULL,&dwSizeOfHeader,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE))
		return 0;
	memcpy((void*)bPEHdr,(const void*)dwModuleAddress,0x1000);
	
	IMAGE_FILE_HEADER* ifh = (IMAGE_FILE_HEADER*)(bPEHdr+sizeof(DWORD)+((PIMAGE_DOS_HEADER)bPEHdr)->e_lfanew);
	IMAGE_OPTIONAL_HEADER* ioh = ((IMAGE_OPTIONAL_HEADER*)(bPEHdr+sizeof(DWORD)+sizeof(IMAGE_FILE_HEADER)+((PIMAGE_DOS_HEADER)bPEHdr)->e_lfanew));
	IMAGE_SECTION_HEADER* section = (IMAGE_SECTION_HEADER*)((bPEHdr+sizeof(DWORD)+sizeof(IMAGE_FILE_HEADER)+((PIMAGE_DOS_HEADER)bPEHdr)->e_lfanew)+sizeof(IMAGE_OPTIONAL_HEADER));
	DWORD dwSectionCount = ifh->NumberOfSections;

	if(dwSectionCount<=0 || dwSectionCount>10)
		return 0;

	BYTE* bOldData = NULL;
	
	if(STATUS_SUCCESS != NtAllocateVirtualMemory(NtCurrentProcess(),(PVOID*)&bOldData,NULL,&ioh->SizeOfImage,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE))
		return 0;

	memcpy((void*)bOldData,(const void*)dwModuleAddress,ioh->SizeOfImage);

	if(STATUS_SUCCESS != NtUnmapViewOfSection(NtCurrentProcess(),(PVOID)dwModuleAddress))
		return 0;

	DWORD dwPeAndTextSize = section[1].VirtualAddress & 0xFFFF0000;
	if(!dwPeAndTextSize)
		return 0;
	DWORD dwTmpAddr = 0;
	HANDLE hSection = 0;
	LARGE_INTEGER liSize = {0}; 
	SIZE_T stViewSize = {0};
	liSize.LowPart = dwPeAndTextSize;
	
	if(STATUS_SUCCESS != NtCreateSection(&hSection,SECTION_ALL_ACCESS,NULL,&liSize,PAGE_EXECUTE_READWRITE,SEC_COMMIT,NULL))
		return 0;

	if(STATUS_SUCCESS != NtMapViewOfSection(hSection,NtCurrentProcess(),(PVOID*)&dwTmpAddr,NULL,NULL,NULL,&stViewSize,ViewShare,NULL,PAGE_EXECUTE_READWRITE))
		return 0;

	DWORD dwUnprotectedStart = dwModuleAddress+dwPeAndTextSize;
	DWORD dwUnprotectedSize = ioh->SizeOfImage-dwPeAndTextSize;

	if(STATUS_SUCCESS != NtAllocateVirtualMemory(NtCurrentProcess(),(PVOID*)&dwUnprotectedStart,NULL,&dwUnprotectedSize,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE))
		return 0;

	memcpy((void*)dwTmpAddr,(const void*)bOldData,dwPeAndTextSize);

	NtUnmapViewOfSection(NtCurrentProcess(),(PVOID)dwTmpAddr);

	NtMapViewOfSection(hSection,NtCurrentProcess(),(PVOID*)&dwModuleAddress,NULL,NULL,NULL,&stViewSize,ViewShare,NULL,PAGE_EXECUTE_READ);

	memcpy((void*)dwUnprotectedStart,(const void*)(bOldData+dwPeAndTextSize),dwUnprotectedSize);

	NtFreeVirtualMemory(NtCurrentProcess(),(PVOID*)&bOldData,&ioh->SizeOfImage,MEM_RELEASE);
	NtFreeVirtualMemory(NtCurrentProcess(),(PVOID*)&bPEHdr,&dwSizeOfHeader,MEM_RELEASE);
#endif
	return 1;
}

DWORD GetModuleChecksum(HMODULE hMod)
{
	IMAGE_FILE_HEADER* ifh = (IMAGE_FILE_HEADER*)((DWORD)hMod+sizeof(DWORD)+((PIMAGE_DOS_HEADER)hMod)->e_lfanew);
	IMAGE_OPTIONAL_HEADER* ioh = ((IMAGE_OPTIONAL_HEADER*)((DWORD)hMod+sizeof(DWORD)+sizeof(IMAGE_FILE_HEADER)+((PIMAGE_DOS_HEADER)hMod)->e_lfanew));
	DWORD res = 0;
	for(DWORD i = (DWORD)hMod + ioh->BaseOfCode; i<((DWORD)hMod + ioh->BaseOfCode + ioh->SizeOfCode); i+= 4)
		res += *(PDWORD)i;
	return res;
}

void CountCCs()
{
	CountCC(dwKernel32);
	CountCC(dwWs2);
	CountCC(dwUser32);
#ifndef K_XRAY
	CountCC(GetModuleHandle(NULL));
#endif
	if(dwKernelBase)
		CountCC(dwKernelBase);
}

void CountCC(HMODULE hMod)
{
	__try
	{
		cp.pair[cp.count].hModule = hMod;
		cp.pair[cp.count].dwChecksum = GetModuleChecksum(cp.pair[cp.count].hModule);
		cp.count++;
	}
	__except(1)
	{
	}
}

int CheckModules()
{
#ifdef ANTIDBG
	for(int i=0;i<cp.count;i++)
	{
		__try
		{
			if(cp.pair[i].dwChecksum != GetModuleChecksum(cp.pair[i].hModule))
			{
	#ifdef DBGPRINT&&ANTIDBG
				printf("%.08X\r\n",cp.pair[i].hModule);
	#endif
				return 0;
			}
		}
		__except(1)
		{
		}
		SwitchToFiber(mainFib);
	}
#endif
	return 1;
}

int CheckModule(HMODULE hMod)
{
#ifdef ANTIDBG
	for(int i=0;i<cp.count;i++)
	{
		if(cp.pair[i].hModule == hMod && cp.pair[i].dwChecksum == GetModuleChecksum(cp.pair[i].hModule))
			return 1;
	}
	return 0;
#else
	return 1;
#endif
}