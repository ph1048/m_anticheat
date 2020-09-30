#include "main.h"
#include "thread.h"
#include "strcrypt.h"
#include "memory.h"

int thread_init()
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

	if(Context.Dr0 || Context.Dr1 || Context.Dr2 || Context.Dr3)
		NtTerminateThread(NtCurrentThread(),-666);

	DWORD dwStartAddr,dwBufflen_out = 0;
	NtQueryInformationThread(NtCurrentThread(), ThreadQuerySetWin32StartAddress, &dwStartAddr, sizeof(dwStartAddr), &dwBufflen_out);

	if(/*initialized && */!isInModule(dwStartAddr))
	{
	if(GetModuleHandle(F("PavTrc.dll")) || GetModuleHandle(F("PavLspHook.dll")) || GetModuleHandle(F("PavLspHookWow.dll")) || GetModuleHandle(F("PavsHook.dll")) || GetModuleHandle(F("PavsHookWow.dll")))
		FatalAppExit(0,"Panda Antivirus is totally incompatible with Magnum Anticheat. It's recommended to completely remove Panda AV.");
		//FatalAppExit(0,"Скорее всего, у вас установлен Panda Antivirus. Для корректной работы игрового приложения необходимо удалить ваше антивирусное ПО. Отключение антивируса или добавление в исключения не устранит проблему.");

		NtTerminateThread(NtCurrentThread(),-666);
	}

	if(dwStartAddr == (DWORD)GetProcAddress(dwNtdll,F("LdrLoadDll")))
	{
		NtTerminateThread(NtCurrentThread(),-666);
	}

	if(dwStartAddr == (DWORD)GetProcAddress(dwNtdll,F("DbgUiRemoteBreakin")))
	{
		NtTerminateThread(NtCurrentThread(),-666);
	}

	if(dwStartAddr == (DWORD)GetProcAddress(dwKernel32,F("LoadLibraryA")))
	{
		NtTerminateThread(NtCurrentThread(),-666);
	}

	if(dwStartAddr == (DWORD)GetProcAddress(dwKernel32,F("LoadLibraryW")))
	{
		NtTerminateThread(NtCurrentThread(),-666);
	}

	if(dwStartAddr == (DWORD)GetProcAddress(dwKernel32,F("LoadLibraryExA")))
	{
		NtTerminateThread(NtCurrentThread(),-666);
	}

	if(dwStartAddr == (DWORD)GetProcAddress(dwKernel32,F("LoadLibraryExW")))
	{
		NtTerminateThread(NtCurrentThread(),-666);
	}

	if(dwKernelBase)
	{
		if(dwStartAddr == (DWORD)GetProcAddress(dwKernelBase,F("LoadLibraryA")))
		{
			NtTerminateThread(NtCurrentThread(),-666);
		}

		if(dwStartAddr == (DWORD)GetProcAddress(dwKernelBase,F("LoadLibraryW")))
		{
			NtTerminateThread(NtCurrentThread(),-666);
		}

		if(dwStartAddr == (DWORD)GetProcAddress(dwKernelBase,F("LoadLibraryExA")))
		{
			NtTerminateThread(NtCurrentThread(),-666);
		}

		if(dwStartAddr == (DWORD)GetProcAddress(dwKernelBase,F("LoadLibraryExW")))
		{
			NtTerminateThread(NtCurrentThread(),-666);
		}
	}
#endif
	return 1;
}