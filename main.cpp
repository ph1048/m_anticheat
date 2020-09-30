#include "main.h"
#include "init.h"
#include "thread.h"

#include "strcrypt.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 ) 
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		return init(hModule);
		break;
	case DLL_THREAD_ATTACH:
		thread_init();
		break;
	case DLL_PROCESS_DETACH:
		NtTerminateProcess(NtCurrentProcess(),-1337);
		break;
	}
	return TRUE;
}