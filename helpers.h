#include <windows.h>
#include <nt/ntdll.h>

int strtrim(char* Dest, char* Src, int size);
DWORD RndDw(DWORD dwVal, DWORD dwBy);
void DoCycleCode(int len);
bool isInModule(DWORD addr);