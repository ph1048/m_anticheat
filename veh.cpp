#include "main.h"
#include "veh.h"
#include "resource.h"

char output[1024] = {0};

INT_PTR CALLBACK DialogProc(
  __in  HWND hwndDlg,
  __in  UINT uMsg,
  __in  WPARAM wParam,
  __in  LPARAM lParam
)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_COMMAND:
			if((wParam&0xFFFF) == IDCANCEL)
				PostQuitMessage(0);
			if((wParam&0xFFFF) == IDC_BUTTON1)
			{
				OpenClipboard(hwndDlg);
				EmptyClipboard();
				HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, strlen(output) + 1);

				LPSTR lpstrCopy = (LPSTR)GlobalLock(hglbCopy);
				memcpy(lpstrCopy, output, strlen(output) + 1);
				lpstrCopy[strlen(output)] = (char)0;
				GlobalUnlock(hglbCopy);

				SetClipboardData(CF_TEXT, hglbCopy);

				CloseClipboard();
			}
			break;

		default:
			return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
	}
	return 0;
}

LONG NTAPI VectoredExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
	HWND error = CreateDialogParam(hDll,MAKEINTRESOURCE(IDD_DIALOG2),NULL,DialogProc,NULL);
	ShowWindow(error, SW_SHOW);
	MSG msg;
	HWND hwndText = GetDlgItem(error,IDC_EDIT1);
	wsprintf(output,\
		"EAX:\t%.08X\r\nEBX:\t%.08X\r\nECX:\t%.08X\r\nEDX:\t%.08X\r\n"
		"ESI:\t%.08X\r\nEDI:\t%.08X\r\nESP:\t%.08X\r\nEBP:\t%.08X\r\n"
		"EIP:\t%.08X\r\nCode:\t%.08X",\
		pExceptionInfo->ContextRecord->Eax,pExceptionInfo->ContextRecord->Ebx,\
		pExceptionInfo->ContextRecord->Ecx,pExceptionInfo->ContextRecord->Edx,\
		pExceptionInfo->ContextRecord->Esi,pExceptionInfo->ContextRecord->Edi,\
		pExceptionInfo->ContextRecord->Esp,pExceptionInfo->ContextRecord->Ebp,\
		pExceptionInfo->ContextRecord->Eip,pExceptionInfo->ExceptionRecord->ExceptionCode);
	SetWindowText(hwndText,output);
	
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	NtTerminateProcess(NtCurrentProcess(),1337);
	return EXCEPTION_CONTINUE_SEARCH;
}

int veh_init()
{
	AddVectoredExceptionHandler(1,VectoredExceptionHandler);
	return 1;
}