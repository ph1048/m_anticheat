#include "init.h"
#include "strcrypt.h"
#include "veh.h"
#include "hooks.h"
#include <Commctrl.h>
#include "resource.h"
#include "hwid.h"
#include <string>

extern int getIDs(char* serial);
extern int GetMonitorSN(char* serial);

NTSTATUS (NTAPI*fnZwRemoveProcessDebug)(HANDLE hProcess, HANDLE hDbgObj);
NTSTATUS (NTAPI*fnZwSetInformationDebugObject)
(HANDLE DebugObject, ULONG InformationClass, PVOID Information, ULONG InformationLength, PULONG ReturnLength);

HHOOK hhookSysMsg;

HWND hwndMainWindow,hwndLoading,hwndPbar = 0;
DWORD dwLoading = 0;
HANDLE hLoading = 0;

HMODULE hDll = 0;

HMODULE dwKernel32 = NULL;
HMODULE dwKernelBase = NULL;
HMODULE dwNtdll = NULL;
HMODULE dwWs2 = NULL;
HMODULE dwUser32 = NULL;

std::wstring cwd;

char HwID[40]={0};

bool initialized = false;

CRITICAL_SECTION csMouseButtons,csLoadWindow;

LRESULT CALLBACK LowLevelKeyboardProc(
  __in  int nCode,
  __in  WPARAM wParam,
  __in  LPARAM lParam
)
{
	if(GetForegroundWindow() != GetActiveWindow())
		return CallNextHookEx(hhookSysMsg,nCode,wParam,lParam);

	if(((KBDLLHOOKSTRUCT *) lParam)->flags & LLKHF_INJECTED)
	{
		return 1;
	}
	return CallNextHookEx(hhookSysMsg,nCode,wParam,lParam);
}

BOOL CreateConsole(void)
{
	FreeConsole();        //на всякий случай
	if( AllocConsole())
	{
		int hCrt = _open_osfhandle((long)
		GetStdHandle(STD_OUTPUT_HANDLE), 0x4000);
		*stdout = *(::_fdopen(hCrt, "w"));
		::setvbuf(stdout, NULL, _IONBF, 0);
		*stderr = *(::_fdopen(hCrt, "w"));
		::setvbuf(stderr, NULL, _IONBF, 0);
		return TRUE;
	}
	return FALSE;
}

UINT ProcessMsgs()
{
	UINT res;
	do
	{
		MSG msg;
		GetMessage(&msg, NULL, 0, 0);
		res = msg.message;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	while(res!=0xf);
	return res;
}

int init(HMODULE hModule)
{
	static bool initx = false;
	if(initx)
		return 1;
	initx = true;

#ifdef DBGPRINT
	CreateConsole();
#endif

	hDll = hModule;

	cwd.clear();
	wchar_t buf[MAX_PATH] = {0};
	GetModuleFileNameW(NULL,buf,sizeof(buf));
	PathRemoveFileSpecW(buf);
	PathAddBackslashW(buf);
	cwd.assign(buf);

	SetCurrentDirectoryW(buf);

#ifdef MIN_DATAGRF_SIZE&&SERVER_TWINKLE
	HANDLE hF = CreateFile("data.grf", GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
	if(hF == INVALID_HANDLE_VALUE)
	{
		FatalAppExit(0,"Файл \"data.grf\" не найден. Продолжение невозможно.");
	}

	if(GetFileSize(hF,NULL)<1288490188 && GetFileSize(hF,NULL)!=INVALID_FILE_SIZE)
	{
		MessageBox(0,"Старая версия data.grf. Во избежание ошибок скачайте полную версию клиента TwinkleRO c торрента.","Предупреждение",0);
	}
	CloseHandle(hF);
#endif

#ifdef SERVER_TWINKLE
	//strcat(GetCommandLineA()," 1rag1");
#endif

	dwKernel32 = GetModuleHandle(F("kernel32.dll"));
	dwKernelBase = GetModuleHandle(F("kernelbase.dll"));
	dwNtdll = GetModuleHandle(F("ntdll.dll"));
	dwWs2 = LoadLibrary(F("ws2_32.dll"));
	dwUser32 = GetModuleHandle(F("user32.dll"));

	if(!(dwKernel32 && dwNtdll && dwWs2 && dwUser32))
		FatalAppExit(0,"The application is incompatible with the system.");
		//FatalAppExit(0,"Приложение несовместимо с системой");

	*(FARPROC*)&fnZwRemoveProcessDebug =			GetProcAddress(dwNtdll,F("ZwRemoveProcessDebug"));
	*(FARPROC*)&fnZwSetInformationDebugObject =		GetProcAddress(dwNtdll,F("ZwSetInformationDebugObject"));
#ifdef ANTIDBG
	HANDLE hDbg = 0;
	DWORD dwRet = 0;
	NtQueryInformationProcess(NtCurrentProcess(),ProcessDebugObjectHandle,(PVOID)&hDbg,sizeof(hDbg),&dwRet);
	DWORD dwState = 1;
	fnZwSetInformationDebugObject(hDbg,1,&dwState,4,0);
	fnZwRemoveProcessDebug(NtCurrentProcess(),hDbg);
#endif

#ifdef DBGPRINT
	printf("1\r\n");
#endif
	

	InitializeCriticalSection(&csMouseButtons);

	hwndLoading = CreateDialogParam(hDll,MAKEINTRESOURCE(IDD_DIALOG1),NULL,NULL,NULL);
	ShowWindow(hwndLoading, SW_SHOW);

	int numItems = 0;

	while(numItems < 3)
	{
		if(ProcessMsgs())
			numItems++;
	}
#ifdef DBGPRINT
	printf("2\r\n");
#endif

	hwndPbar = GetDlgItem(hwndLoading,IDC_PROGRESS1);
	SendMessage(hwndPbar, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); 
	SendMessage(hwndPbar, PBM_SETBKCOLOR,0,RGB(255,255,255));
	SendMessage(hwndPbar, PBM_SETBARCOLOR,0,RGB(30,30,30));
    SendMessage(hwndPbar, PBM_SETSTEP, (WPARAM) 20, 0);

	SendMessage(hwndPbar, PBM_STEPIT, 0, 0);

	//veh_init();

#ifdef CLICKERS
	RAWINPUTDEVICE Rid[2];

	// Keyboard
	Rid[0].usUsagePage = 1;
	Rid[0].usUsage = 6;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget=NULL;

	// Mouse
	Rid[1].usUsagePage = 1;
	Rid[1].usUsage = 2;
	Rid[1].dwFlags = 0;
	Rid[1].hwndTarget=NULL;

	RegisterRawInputDevices(Rid,2,sizeof(RAWINPUTDEVICE));
#endif

	#ifdef DBGPRINT
	printf("3\r\n");
#endif

	SendMessage(hwndPbar, PBM_STEPIT, 0, 0);

	hooks_init();

#ifdef DBGPRINT
	printf("4\r\n");
#endif

	SendMessage(hwndPbar, PBM_STEPIT, 0, 0);

	GetHWID(HwID);

	SendMessage(hwndPbar, PBM_STEPIT, 0, 0);

	SendMessage(hwndPbar, PBM_STEPIT, 0, 0);

	//VMProtectEnd();

	return 1;
}