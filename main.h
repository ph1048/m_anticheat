#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#include <Iphlpapi.h>
#include <nt/ntdll.h>
#include <detours.h>
#include <io.h>
#include "crypto.h"
#include "helpers.h"
#include <VMProtectSDK.h>
#include <Shlwapi.h>

extern HWND hwndLoading,hwndMainWindow,hwndPbar;
extern char HwID[40];
extern DWORD dwMouseState;
extern bool bMouseActive;
extern WORD VirtualKeyboard[255];
extern CRITICAL_SECTION csMouseButtons;
extern NTSTATUS (NTAPI*fnZwRemoveProcessDebug)(HANDLE hProcess, HANDLE hDbgObj);
extern NTSTATUS (NTAPI*fnZwSetInformationDebugObject)
(HANDLE DebugObject, ULONG InformationClass, PVOID Information, ULONG InformationLength, PULONG ReturnLength);

#define IMAGEBASE 0x400000
extern HMODULE hDll;
extern bool initialized;

extern HMODULE dwKernel32;
extern HMODULE dwKernelBase;
extern HMODULE dwNtdll;
extern HMODULE dwWs2;
extern HMODULE dwUser32;
extern unsigned long mapinfo_flags;
extern LPVOID mainFib,fib;

#define F//(s) //VMProtectDecryptStringA(s)// f<sizeof(s)>(s)

#include "config.h"


#define _CRT_SECURE_NO_WARNINGS