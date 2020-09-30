#include "main.h"

int ProtectModule(DWORD dwModuleAddress);
DWORD GetModuleChecksum(HMODULE hMod);
void CountCCs();
void CountCC(HMODULE hMod);
int CheckModules();
int CheckModule(HMODULE hMod);