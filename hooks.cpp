#include "hooks.h"
#include "strcrypt.h"
#include "thread.h"
#include "xdinput.h"
#include "ddraw.h"
#include "crypto.h"
#include <string>
#include "features.h"
#include "memory.h"
#include "MD5Checksum.h"
#include "SocketStream.h"

using namespace std;

DWORD dwEntryAddr = 0;

char dataini[10][64];

char exemd5[33] = {0};

UCHAR tick_i = 0;
UCHAR tick_o = 0;

SOCKET current_network = -1;
SOCKET webbrowser = -1;
SOCKET active_socket = -1;

PIMAGE_NT_HEADERS
(NTAPI*
orig_RtlImageNtHeader)(PVOID Base);
BOOL
(WINAPI*
orig_PeekMessageA)(
    __out LPMSG lpMsg,
    __in_opt HWND hWnd,
    __in UINT wMsgFilterMin,
    __in UINT wMsgFilterMax,
    __in UINT wRemoveMsg);

HRESULT (WINAPI* orig_DirectInputCreateA)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA *ppDI, LPUNKNOWN punkOuter);
HWND
(WINAPI*
orig_CreateWindowExA)(
    __in DWORD dwExStyle,
    __in_opt LPCSTR lpClassName,
    __in_opt LPCSTR lpWindowName,
    __in DWORD dwStyle,
    __in int X,
    __in int Y,
    __in int nWidth,
    __in int nHeight,
    __in_opt HWND hWndParent,
    __in_opt HMENU hMenu,
    __in_opt HINSTANCE hInstance,
    __in_opt LPVOID lpParam);

BOOL
(WINAPI*
orig_RegisterRawInputDevices)(
    __in_ecount(uiNumDevices) PCRAWINPUTDEVICE pRawInputDevices,
    __in UINT uiNumDevices,
    __in UINT cbSize);

int (WINAPI* orig_send)(SOCKET s, char* buf, int len, int flags);
int (WINAPI* orig_recv)(SOCKET s, char* buf, int len, int flags);
int (PASCAL* orig_select)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout);
HANDLE
(WINAPI*
orig_CreateFileA)(
    __in     LPCSTR lpFileName,
    __in     DWORD dwDesiredAccess,
    __in     DWORD dwShareMode,
    __in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    __in     DWORD dwCreationDisposition,
    __in     DWORD dwFlagsAndAttributes,
    __in_opt HANDLE hTemplateFile
    );

int (PASCAL* orig_connect) (
                        IN SOCKET s,
                        IN const struct sockaddr FAR *name,
                        IN int namelen);

DWORD
(WINAPI*
orig_GetPrivateProfileStringA)(LPCSTR lpAppName,LPCSTR lpKeyName,LPCSTR lpDefault,LPSTR lpReturnedString,DWORD nSize,LPCSTR lpFileName);

LRESULT (CALLBACK* orig_WindowProc)(
  __in  HWND hwnd,
  __in  UINT uMsg,
  __in  WPARAM wParam,
  __in  LPARAM lParam
);
ATOM (WINAPI* orig_RegisterClass)(
  __in  const WNDCLASS *lpWndClass
);

HRESULT (WINAPI* orig_DirectDrawCreateEx)( GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter );


void* (__fastcall* orig_GetData)(DWORD _this, DWORD edx, char* name, unsigned long* size);
DWORD (__fastcall*RagnarokMessage)(DWORD _this, DWORD edx, char* text, DWORD p1, DWORD p2, DWORD p3, DWORD p4, char* caption);





LPVOID mainFib,fib;

void Guardian()
{
	while(1)
	{
		if(!CheckModules())
		{
			initialized = false;
			FatalAppExit(0,"Memory corruption");
			ExitProcess(0);
		}
	}
}

NTSTATUS
NTAPI
_RtlImageNtHeaderEx(
    ULONG Flags,
    PVOID Base,
    ULONG64 Size,
    OUT PIMAGE_NT_HEADERS * OutHeaders
    )
{
    PIMAGE_NT_HEADERS NtHeaders = 0;
    ULONG e_lfanew = 0;
    BOOLEAN RangeCheck = 0;
    NTSTATUS Status = 0;
    const ULONG ValidFlags = 
        RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK;

    if (OutHeaders != NULL) {
        *OutHeaders = NULL;
    }
    if (OutHeaders == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if ((Flags & ~ValidFlags) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (Base == NULL || Base == (PVOID)(LONG_PTR)-1) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    RangeCheck = ((Flags & RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK) == 0);
    if (RangeCheck) {
        if (Size < sizeof(IMAGE_DOS_HEADER)) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }
    }

    //
    // Exception handling is not available in the boot loader, and exceptions
    // were not historically caught here in kernel mode. Drivers are considered
    // trusted, so we can't get an exception here due to a bad file, but we
    // could take an inpage error.
    //
#define EXIT goto Exit
   /* if (((PIMAGE_DOS_HEADER)Base)->e_magic != IMAGE_DOS_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        EXIT;
    }*/
    e_lfanew = ((PIMAGE_DOS_HEADER)Base)->e_lfanew;
    if (RangeCheck) {
        if (e_lfanew >= Size
#define SIZEOF_PE_SIGNATURE 4
            || e_lfanew >= (MAXULONG - SIZEOF_PE_SIGNATURE - sizeof(IMAGE_FILE_HEADER))
            || (e_lfanew + SIZEOF_PE_SIGNATURE + sizeof(IMAGE_FILE_HEADER)) >= Size
            ) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            EXIT;
        }
    }

    NtHeaders = (PIMAGE_NT_HEADERS)((PCHAR)Base + e_lfanew);

    //
    // In kernelmode, do not cross from usermode address to kernelmode address.
    //
    if ((DWORD)Base < MM_HIGHEST_USER_ADDRESS) {
        if ((DWORD)NtHeaders >= MM_HIGHEST_USER_ADDRESS) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            EXIT;
        }
        //
        // Note that this check is slightly overeager since IMAGE_NT_HEADERS has
        // a builtin array of data_directories that may be larger than the image
        // actually has. A better check would be to add FileHeader.SizeOfOptionalHeader,
        // after ensuring that the FileHeader does not cross the u/k boundary.
        //
        if ((DWORD)((PCHAR)NtHeaders + sizeof (IMAGE_NT_HEADERS)) >= MM_HIGHEST_USER_ADDRESS) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            EXIT;
        }
    }

    if (NtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        EXIT;
    }
    Status = STATUS_SUCCESS;

Exit:
    if (NT_SUCCESS(Status)) {
        *OutHeaders = NtHeaders;
    }
    return Status;
}
#undef EXIT

PIMAGE_NT_HEADERS
NTAPI
my_RtlImageNtHeader(
    PVOID Base
    )
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    (VOID)_RtlImageNtHeaderEx(RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, Base, 0, &NtHeaders);
    return NtHeaders;
}

BOOL
WINAPI
my_RegisterRawInputDevices(
    __in_ecount(uiNumDevices) PCRAWINPUTDEVICE pRawInputDevices,
    __in UINT uiNumDevices,
    __in UINT cbSize)
{
	return 0;
}

struct KeyDownInfo
{
	unsigned RepeatCount:16;
	unsigned ScanCode:8;
	unsigned Extended:1;
	unsigned Reserved:4;
	unsigned ContextCode:1;
	unsigned PreviousKeyState:1;
	unsigned TransitionState:1;
};

WORD VirtualKeyboard[255] = {0};
DWORD dwMouseState = 0;
bool bMouseActive = true;

LPBYTE lpb = 0;
DWORD dwBufLen = 0;

BOOL
WINAPI
my_PeekMessageA(
    __out LPMSG lpMsg,
    __in_opt HWND hWnd,
    __in UINT wMsgFilterMin,
    __in UINT wMsgFilterMax,
    __in UINT wRemoveMsg)
{
startpeek:
	BOOL resu = orig_PeekMessageA(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,wRemoveMsg);
	if(!resu)
		return 0;//нету ничего
	if(lpMsg->message == WM_INPUT)
		goto wmin;
positiveresult:
	if(lpMsg->message == WM_KEYUP || lpMsg->message == WM_KEYDOWN || lpMsg->message == WM_SYSKEYUP || lpMsg->message == WM_SYSKEYDOWN)
	{//перехваты
		resu = orig_PeekMessageA(lpMsg,hWnd,WM_INPUT,WM_INPUT,1);
		if(!resu)
			return 0;
		if(lpMsg->message == WM_INPUT)
			goto wmin;
		else
			goto positiveresult;
	}
	else
		goto obrabotka;

wmin:

	UINT dwSize = 0;

	GetRawInputData((HRAWINPUT)lpMsg->lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	if(dwSize>dwBufLen)
	{
		if(lpb)
			delete lpb;
		lpb = new BYTE[dwSize];
		dwBufLen = dwSize;
	}

	if (lpb == NULL)
		return 0;

	GetRawInputData((HRAWINPUT)lpMsg->lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

	RAWINPUT* raw = (RAWINPUT*)lpb;

#ifndef CLICKERS_S
	if(!raw->header.hDevice && !(mapinfo_flags&1))
		return 0;
#endif

	if (raw->header.dwType == RIM_TYPEKEYBOARD) 
	{
		if(!(raw->data.keyboard.Message == WM_KEYUP || raw->data.keyboard.Message == WM_KEYDOWN ||
			raw->data.keyboard.Message == WM_SYSKEYUP || raw->data.keyboard.Message == WM_SYSKEYDOWN))
			return 0;

		lpMsg->hwnd = GetActiveWindow();
		lpMsg->message = raw->data.keyboard.Message;
		lpMsg->wParam = raw->data.keyboard.VKey;

		((KeyDownInfo*)&lpMsg->lParam)->ScanCode = raw->data.keyboard.MakeCode;
		((KeyDownInfo*)&lpMsg->lParam)->Extended = 0;
		((KeyDownInfo*)&lpMsg->lParam)->Reserved = 0;

		if(lpMsg->message == WM_KEYDOWN)
		{
			((KeyDownInfo*)&lpMsg->lParam)->ContextCode = 0;
			((KeyDownInfo*)&lpMsg->lParam)->PreviousKeyState = VirtualKeyboard[raw->data.keyboard.VKey]?1:0;
			((KeyDownInfo*)&lpMsg->lParam)->TransitionState = 0;
			((KeyDownInfo*)&lpMsg->lParam)->RepeatCount = ++VirtualKeyboard[raw->data.keyboard.VKey];
		}

		if(lpMsg->message == WM_SYSKEYDOWN)
		{
			if(VirtualKeyboard[VK_MENU])
				((KeyDownInfo*)&lpMsg->lParam)->ContextCode = 1;
			else
				((KeyDownInfo*)&lpMsg->lParam)->ContextCode = 0;
			((KeyDownInfo*)&lpMsg->lParam)->PreviousKeyState = VirtualKeyboard[raw->data.keyboard.VKey]?1:0;
			((KeyDownInfo*)&lpMsg->lParam)->TransitionState = 0;
			((KeyDownInfo*)&lpMsg->lParam)->RepeatCount = ++VirtualKeyboard[raw->data.keyboard.VKey];
		}

		if(lpMsg->message == WM_KEYUP)
		{
			((KeyDownInfo*)&lpMsg->lParam)->ContextCode = 0;
			((KeyDownInfo*)&lpMsg->lParam)->PreviousKeyState = 1;
			((KeyDownInfo*)&lpMsg->lParam)->TransitionState = 1;
			((KeyDownInfo*)&lpMsg->lParam)->RepeatCount = 1;
			VirtualKeyboard[raw->data.keyboard.VKey] = 0;
		}

		if(lpMsg->message == WM_SYSKEYUP)
		{
			if(VirtualKeyboard[VK_MENU])
				((KeyDownInfo*)&lpMsg->lParam)->ContextCode = 1;
			else
				((KeyDownInfo*)&lpMsg->lParam)->ContextCode = 0;
			((KeyDownInfo*)&lpMsg->lParam)->PreviousKeyState = 1;
			((KeyDownInfo*)&lpMsg->lParam)->TransitionState = 1;
			((KeyDownInfo*)&lpMsg->lParam)->RepeatCount = 1;
			VirtualKeyboard[raw->data.keyboard.VKey] = 0;
		}
	}
	else if(raw->header.dwType == RIM_TYPEMOUSE)
	{
		if(raw->data.mouse.usButtonFlags&RI_MOUSE_LEFT_BUTTON_DOWN)
		{
			((PBYTE)&dwMouseState)[0] = 0x80;
		}
		if(raw->data.mouse.usButtonFlags&RI_MOUSE_LEFT_BUTTON_UP)
		{
			((PBYTE)&dwMouseState)[0] = 0x00;
		}
		if(raw->data.mouse.usButtonFlags&RI_MOUSE_RIGHT_BUTTON_DOWN)
		{
			((PBYTE)&dwMouseState)[1] = 0x80;
		}
		if(raw->data.mouse.usButtonFlags&RI_MOUSE_RIGHT_BUTTON_UP)
		{
			((PBYTE)&dwMouseState)[1] = 0x00;
		}
		if(raw->data.mouse.usButtonFlags&RI_MOUSE_MIDDLE_BUTTON_DOWN)
		{
			((PBYTE)&dwMouseState)[2] = 0x80;
		}
		if(raw->data.mouse.usButtonFlags&RI_MOUSE_MIDDLE_BUTTON_UP)
		{
			((PBYTE)&dwMouseState)[2] = 0x00;
		}
	}
	else
		return 0;
obrabotka:
	if(lpMsg->message == WM_KEYUP || lpMsg->message == WM_KEYDOWN || lpMsg->message == WM_SYSKEYUP || lpMsg->message == WM_SYSKEYDOWN)
	{
		TranslateMessage(lpMsg);
		orig_WindowProc(lpMsg->hwnd,lpMsg->message,lpMsg->wParam,lpMsg->lParam);
		return 0;
	}
	else
		return 1;
}

HRESULT WINAPI my_DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA *ppDI, LPUNKNOWN punkOuter)
{
	HRESULT hr = orig_DirectInputCreateA(hinst,dwVersion,ppDI,punkOuter);

	if(SUCCEEDED(hr))
		*ppDI = new xDirectInput(reinterpret_cast<IDirectInput*>(*ppDI));

	return hr;
}

HWND
WINAPI
my_CreateWindowExA(
    __in DWORD dwExStyle,
    __in_opt LPCSTR lpClassName,
    __in_opt LPCSTR lpWindowName,
    __in DWORD dwStyle,
    __in int X,
    __in int Y,
    __in int nWidth,
    __in int nHeight,
    __in_opt HWND hWndParent,
    __in_opt HMENU hMenu,
    __in_opt HINSTANCE hInstance,
    __in_opt LPVOID lpParam)
{
	if(!initialized)
	{
		char szCaption[128] = "";
		if(lpWindowName)
			strcat(szCaption,lpWindowName);
		strcat(szCaption,F(" [*Magnum Anti-Cheat*]"));
		CountCCs();
		initialized = true;
		HWND hwndWnd = orig_CreateWindowExA(dwExStyle,lpClassName,szCaption,dwStyle,X,Y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);
		EndDialog(hwndLoading, 0);
		fib = CreateFiber(0,(LPFIBER_START_ROUTINE)Guardian,NULL);
		mainFib = ConvertThreadToFiber(NULL);
		//CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)Guardian,NULL,NULL,NULL);
		/*HMODULE x = LoadLibrary(".\\RCX\\RCXDraw.dll");
		void (__stdcall*f1)();void (__stdcall*f2)();
		*(FARPROC*)&f1 = GetProcAddress(x,"HookProc");
		*(FARPROC*)&f2 = GetProcAddress(x,"InstallHook");
		//
		f2();f1();
		//SetWindowsHookExA(WH_CBT,(HOOKPROC)f1,x,0);*/
		return hwndWnd;
	}
	else
		return orig_CreateWindowExA(dwExStyle,lpClassName,lpWindowName,dwStyle,X,Y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);
}

int WINAPI my_send(SOCKET s, char* buf, int len, int flags)
{
#ifdef ANTIDBG
	HANDLE hDbg = 0;
	DWORD dwRet = 0;
	NtQueryInformationProcess(NtCurrentProcess(),ProcessDebugObjectHandle,(PVOID)&hDbg,sizeof(hDbg),&dwRet);
	DWORD dwState = 1;
	fnZwSetInformationDebugObject(hDbg,1,&dwState,4,0);
	fnZwRemoveProcessDebug(NtCurrentProcess(),hDbg);

	CONTEXT Context = {0};
	Context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(GetCurrentThread(), &Context);

	DWORD dwRetAddr = 0;

	__asm
	{
		push eax
		mov eax,[ebp+4]
		mov dwRetAddr,eax
		pop eax
	}

	DWORD addr;
	NtQueryInformationThread(NtCurrentThread(),ThreadQuerySetWin32StartAddress,(PVOID)&addr,4,NULL);

	DWORD codeBaseTop = IMAGEBASE+orig_RtlImageNtHeader((PVOID)IMAGEBASE)->OptionalHeader.BaseOfCode+orig_RtlImageNtHeader((PVOID)IMAGEBASE)->OptionalHeader.SizeOfCode;
	DWORD codeBase = IMAGEBASE+orig_RtlImageNtHeader((PVOID)IMAGEBASE)->OptionalHeader.BaseOfCode;

	SwitchToFiber(fib);

	if(dwRetAddr>codeBaseTop || dwRetAddr<codeBase || Context.Dr0 || Context.Dr1 || Context.Dr2 || Context.Dr3 || !initialized || addr != (orig_RtlImageNtHeader((PVOID)IMAGEBASE)->OptionalHeader.AddressOfEntryPoint+IMAGEBASE))
	{
		return SOCKET_ERROR;
	}
#endif
	switch(*(PWORD)buf)
	{
	case 0x825:
		{// <2b header>.<24b login>.<32b token>.<32b hwid>.<32b exe>
			char sNewLoginPacket[124] = {0};
			*(PWORD)&sNewLoginPacket[0] = 0x1337;
			memcpy((void*)&sNewLoginPacket[2],(const void*)&buf[0x9],23);
			memcpy((void*)&sNewLoginPacket[26],(const void*)&buf[0x5c],32);
			memcpy((void*)&sNewLoginPacket[59],(const void*)HwID,32);
			memcpy((void*)&sNewLoginPacket[92],(const void*)exemd5,32);
			PosEncryptNet((UCHAR*)sNewLoginPacket,sizeof(sNewLoginPacket),&tick_o);
			orig_send(current_network, sNewLoginPacket, sizeof(sNewLoginPacket), flags);
			return len;
		}
		break;
	case 0x64:
		{
			char sNewLoginPacket2[87] = {0};
			memcpy((void*)sNewLoginPacket2,(const void*)buf,55);
			memcpy((void*)&sNewLoginPacket2[55],(const void*)HwID,32);
			PosEncryptNet((UCHAR*)sNewLoginPacket2,sizeof(sNewLoginPacket2),&tick_o);
			orig_send(current_network, sNewLoginPacket2, sizeof(sNewLoginPacket2), flags);
			return len;
		}
		break;
	}

	PosEncryptNet((UCHAR*)buf,len,&tick_o);

	return orig_send(current_network, buf, len, flags);
}

char cRecvBuf[4096];
CSocketStream RecvData;

int WINAPI my_recv(SOCKET s, char* buf, int len, int flags)
{
#ifdef ANTIDBG
	CONTEXT Context = {0};
	Context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(GetCurrentThread(), &Context);

	DWORD dwRetAddr = 0;

	__asm
	{
		push eax
		mov eax,[ebp+4]
		mov dwRetAddr,eax
		pop eax
	}

	DWORD addr;
	NtQueryInformationThread(NtCurrentThread(),ThreadQuerySetWin32StartAddress,(PVOID)&addr,4,NULL);

	DWORD codeBaseTop = IMAGEBASE+orig_RtlImageNtHeader((PVOID)IMAGEBASE)->OptionalHeader.BaseOfCode+orig_RtlImageNtHeader((PVOID)IMAGEBASE)->OptionalHeader.SizeOfCode;
	DWORD codeBase = IMAGEBASE+orig_RtlImageNtHeader((PVOID)IMAGEBASE)->OptionalHeader.BaseOfCode;
	
	if(dwRetAddr>codeBaseTop || !initialized || dwRetAddr<codeBase || Context.Dr0 || Context.Dr1 || Context.Dr2 || Context.Dr3 || addr != (orig_RtlImageNtHeader((PVOID)IMAGEBASE)->OptionalHeader.AddressOfEntryPoint+IMAGEBASE))
	{
		return SOCKET_ERROR;
	}
#endif
	

	return RecvData.RecvToClient((UCHAR*)buf,(ULONG)len);
}

int PASCAL FAR my_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout)
{
#ifdef ANTIDBG
	if(!initialized)
		return SOCKET_ERROR;
	CONTEXT Context = {0};
	Context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(GetCurrentThread(), &Context);

	if(Context.Dr0 || Context.Dr1 || Context.Dr2 || Context.Dr3)
		return 0;
#endif

	if(readfds)
	{
		int res = orig_select(nfds, readfds, writefds, exceptfds, timeout);
		if(res>0 && readfds->fd_count>0 && readfds->fd_array[0] == current_network)
		{
			int len = orig_recv(current_network, cRecvBuf, 2048, 0);

			if(len<=0)
			{
				WSASetLastError(WSAENOTSOCK);
				RecvData.Reset();
				return SOCKET_ERROR;
			}

			RecvData.RecvFromServer((UCHAR*)cRecvBuf,(ULONG)len);	

			if(RecvData.HasSomethingToRead())
			{
				return res;
			}
			else
			{
				readfds->fd_count = 0;
				return 0;
			}
		}
		else if(RecvData.HasSomethingToRead())
		{
			readfds->fd_array[0] = current_network;
			readfds->fd_count = 1;
			return 1;
		}
		return res;
	}
	return orig_select(nfds, readfds, writefds, exceptfds, timeout);
}

void* __fastcall my_GetData(DWORD _this, DWORD edx, char* name, unsigned long* size)
{
	DWORD res = (DWORD)orig_GetData(_this, edx, name, size);

#ifdef DBGPRINT
	if(!res)
	printf("%.08X %s\r\n",res,name);
#endif

	if(!res && strstr(name,".bmp") && !strstr(name,"emblem"))
		return orig_GetData(_this, edx, "data\\null.bmp", size);

	if(!res && strstr(name,".spr"))
		return orig_GetData(_this, edx, "data\\null.spr", size);

	if(!res && strstr(name,".pal"))
		return orig_GetData(_this, edx, "data\\null.pal", size);

	if(!res && strstr(name,".act"))
		return orig_GetData(_this, edx, "data\\null.act", size);

	if(*size && res && *(PDWORD)res == 'niwT' && *(PDWORD)(res+4) == 'elk')
	{
		unsigned char p = 0;
		unsigned char check = 0;
		int i = 0;
		for(int pos = 9;pos<=*size;pos++)
		{
			check += (*(PBYTE)(res+pos))*(++i&3);
		}
		if((*(PBYTE)(res+8) & 0xff) == (check & 0xff))
		{
			PosDecrypt((unsigned char*)(res+9),*size-9,&p);
		}
		else
			return (void*)res;
		for(int pos = 9;pos<=*size;pos++)
		{
			*(PBYTE)(res+pos-9) = *(PBYTE)(res+pos);
		}
		ZeroMemory((void*)(res+(*size)-9),9);
		*size -= 9;
	}
	return (void*)res;
}

HANDLE
WINAPI
my_CreateFileA(
    __in     LPCSTR lpFileName,
    __in     DWORD dwDesiredAccess,
    __in     DWORD dwShareMode,
    __in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    __in     DWORD dwCreationDisposition,
    __in     DWORD dwFlagsAndAttributes,
    __in_opt HANDLE hTemplateFile
    )
{
	char path[MAX_PATH] = {0};
	strcpy_s(path,MAX_PATH,lpFileName);
	_strlwr_s(path,MAX_PATH);
	
	if(strstr(path,F(".act")) || strstr(path,F(".spr")))
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
	}

	return orig_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

DWORD
WINAPI
my_GetPrivateProfileStringA(LPCSTR lpAppName,LPCSTR lpKeyName,LPCSTR lpDefault,LPSTR lpReturnedString,DWORD nSize,LPCSTR lpFileName)
{
	if(!strcmp(lpAppName,F("Data")))
	{
		int number = atoi(lpKeyName);
		if(number < 0 || number > 9)
			return NULL;

		if(strlen(dataini[number])<=0)
			return NULL;

		strcpy(lpReturnedString,dataini[number]);

		return strlen(dataini[number]);
	}
	return orig_GetPrivateProfileStringA(lpAppName,lpKeyName,lpDefault,lpReturnedString,nSize,lpFileName);
}

int PASCAL FAR my_connect (
                        IN SOCKET s,
                        IN const struct sockaddr FAR *name,
                        IN int namelen)
{
	static ULONG proxyip = 0;
	tick_i = 0;
	tick_o = 0;
#ifdef TW
	if(!proxyip && ((sockaddr_in*)name)->sin_addr.S_un.S_addr==inet_addr("178.63.130.131") ||
		((sockaddr_in*)name)->sin_addr.S_un.S_addr==inet_addr("176.9.23.26"))
		proxyip = ((sockaddr_in*)name)->sin_addr.S_un.S_addr;
#endif
#ifdef ANTIDBG
	if(!initialized || ( ((sockaddr_in*)name)->sin_addr.S_un.S_addr!=inet_addr(F(SERVER_IP))
#ifdef TW
		&& !proxyip
#endif
		))
	{
		current_network = -1;
		SetLastError(WSAENOTSOCK);
		return SOCKET_ERROR;
	}
#endif
	RecvData.Reset();
	current_network = s;
#ifdef TW
	if(proxyip)
		((sockaddr_in*)name)->sin_addr.S_un.S_addr=proxyip;
#endif
	printf("%.08X %d\r\n",((sockaddr_in*)name)->sin_addr.S_un.S_addr,ntohs(((sockaddr_in*)name)->sin_port));
	return orig_connect(s,name,namelen);
}

LRESULT CALLBACK my_WindowProc(
  __in  HWND hwnd,
  __in  UINT uMsg,
  __in  WPARAM wParam,
  __in  LPARAM lParam
)
{
	if(uMsg==WM_KEYDOWN || uMsg==WM_KEYUP || uMsg==WM_SYSKEYDOWN || uMsg==WM_SYSKEYUP)
		return 1;

	return orig_WindowProc(hwnd, uMsg, wParam, lParam);
}

ATOM WINAPI my_RegisterClass(WNDCLASS *lpWndClass)
{
	*(FARPROC*)&orig_WindowProc = (FARPROC)lpWndClass->lpfnWndProc;
	lpWndClass->lpfnWndProc = (WNDPROC)my_WindowProc;
	return orig_RegisterClass(lpWndClass);
}

HRESULT WINAPI my_DirectDrawCreateEx( GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter )
{
	HRESULT hr = orig_DirectDrawCreateEx(lpGuid, lplpDD, iid, pUnkOuter);

	if(SUCCEEDED(hr))
	{
		printf("my_DirectDrawCreateEx ok\r\n");
		*lplpDD = new xDirectDraw(reinterpret_cast<IDirectDraw7*>(*lplpDD));
	}
	return hr;
}

int hooks_init()
{
	*(FARPROC*)&orig_RtlImageNtHeader			= GetProcAddress(dwNtdll,	F("RtlImageNtHeader"));
	*(FARPROC*)&orig_PeekMessageA				= GetProcAddress(dwUser32,	F("PeekMessageA"));
	*(FARPROC*)&orig_CreateWindowExA			= GetProcAddress(dwUser32,	F("CreateWindowExA"));
	*(FARPROC*)&orig_RegisterRawInputDevices	= GetProcAddress(dwUser32,	F("RegisterRawInputDevices"));
	*(FARPROC*)&orig_RegisterClass				= GetProcAddress(dwUser32,	F("RegisterClassA"));
	*(FARPROC*)&orig_DirectInputCreateA			= GetProcAddress(LoadLibraryA(F("dinput.dll")),	F("DirectInputCreateA"));
	*(FARPROC*)&orig_DirectDrawCreateEx			= GetProcAddress(LoadLibraryA(F("DDRAW.dll")),	F("DirectDrawCreateEx"));
	*(FARPROC*)&orig_send						= GetProcAddress(dwWs2,	F("send"));
	*(FARPROC*)&orig_recv						= GetProcAddress(dwWs2,	F("recv"));
	*(FARPROC*)&orig_select						= GetProcAddress(dwWs2,	F("select"));
	*(FARPROC*)&orig_connect					= GetProcAddress(dwWs2,	F("connect"));
	*(FARPROC*)&orig_CreateFileA				= GetProcAddress(dwKernel32,F("CreateFileA"));
	*(FARPROC*)&orig_GetPrivateProfileStringA	= GetProcAddress(dwKernel32,F("GetPrivateProfileStringA"));
	*(DWORD*)&orig_GetData						= 0x574BD0;

	//*(DWORD*)&RagnarokMessage = 0x00528F70;
	//RagnarokMessage(0x82D7F8,0,"text",0,0,0,0,"caption");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)orig_CreateWindowExA,				my_CreateWindowExA);
	DetourAttach(&(PVOID&)orig_send,						my_send);
	DetourAttach(&(PVOID&)orig_recv,						my_recv);
	DetourAttach(&(PVOID&)orig_select,						my_select);
	DetourAttach(&(PVOID&)orig_connect,						my_connect);
	DetourAttach(&(PVOID&)orig_CreateFileA,					my_CreateFileA);
	DetourAttach(&(PVOID&)orig_GetPrivateProfileStringA,	my_GetPrivateProfileStringA);
	//DetourAttach(&(PVOID&)orig_DirectDrawCreateEx,			my_DirectDrawCreateEx);

#ifdef CLICKERS
	DetourAttach(&(PVOID&)orig_RegisterClass,				my_RegisterClass);
	DetourAttach(&(PVOID&)orig_PeekMessageA,				my_PeekMessageA);
	DetourAttach(&(PVOID&)orig_RegisterRawInputDevices,		my_RegisterRawInputDevices);
	DetourAttach(&(PVOID&)orig_DirectInputCreateA,			my_DirectInputCreateA);
#endif

#ifdef TW
	DetourAttach(&(PVOID&)orig_GetData,						my_GetData);
#endif

	DetourTransactionCommit();

	wchar_t fname[MAX_PATH] = {0};
	GetModuleFileNameW(GetModuleHandle(NULL),fname,sizeof(fname));

#ifdef DATAINI
	int grfnum = 0;
	strcpy(dataini[grfnum++],F("magnum.grf"));
#ifdef GRF1
	strcpy(dataini[grfnum++],GRF1);
#endif
#ifdef GRF2
	strcpy(dataini[grfnum++],GRF2);
#endif
#ifdef GRF3
	strcpy(dataini[grfnum++],GRF3);
#endif
#ifdef GRF4
	strcpy(dataini[grfnum++],GRF4);
#endif
#ifdef GRF5
	strcpy(dataini[grfnum++],GRF5);
#endif
#ifdef GRF6
	strcpy(dataini[grfnum++],GRF6);
#endif
#ifdef GRF7
	strcpy(dataini[grfnum++],GRF7);
#endif
#ifdef GRF8
	strcpy(dataini[grfnum++],GRF8);
#endif
#ifdef GRF9
	strcpy(dataini[grfnum++],GRF9);
#endif

#else
	strcpy(dataini[0],F("magnum.grf"));
	for(int i=0,j=1;i<=9;i++)
	{
		char buf[8] = {0};
		char outbuf[64] = {0};
		itoa(i,buf,10);
		if(orig_GetPrivateProfileStringA(F("Data"),buf,NULL,outbuf,sizeof(outbuf),F(".\\DATA.INI")) && strnlen(outbuf,sizeof(outbuf))>0 && strnlen(outbuf,sizeof(outbuf))<64)
			strcpy(dataini[j++],outbuf);
		if(j>9)
			break;
	}
#endif

	strcpy(exemd5,CMD5Checksum::GetMD5((LPCWSTR)fname));

	if(
#ifdef EXE_MD5
		strcmp(exemd5,F(EXE_MD5)) ||
#endif
#ifndef GRF_MD5
		strcmp(CMD5Checksum::GetMD5((LPCSTR)F("magnum.grf")),F("dc8935168a208ec2a3fc9ab84ee2b444"))
#else
		strcmp(CMD5Checksum::GetMD5((LPCSTR)F("magnum.grf")),GRF_MD5)
#endif
#ifdef ADD_GRF_CHECK
		ADD_GRF_CHECK
#endif
		)
	{
		FatalAppExit(0,"Some of the files were modified by virus or something.");
		NtTerminateProcess(NtCurrentProcess(),1337);
	}

	return 1;
}