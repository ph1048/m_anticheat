#include "main.h"

#define MM_HIGHEST_USER_ADDRESS 0x7FFFEFFF

NTSTATUS
NTAPI
_RtlImageNtHeaderEx(
    ULONG Flags,
    PVOID Base,
    ULONG64 Size,
    OUT PIMAGE_NT_HEADERS * OutHeaders
    );

PIMAGE_NT_HEADERS
NTAPI
my_RtlImageNtHeader(
    PVOID Base
    );

NTSTATUS NTAPI my_ZwContinue(PCONTEXT ThreadContext, BOOLEAN RaiseAlert);
//void my_ZwContinue();

int hooks_init();