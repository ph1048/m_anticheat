#include "main.h"

int parse_packet(UCHAR* buf,UINT32 len,int* remove);
int get_id_name_color(UINT32 id,UINT32* fg,UINT32* bg);
void __stdcall update_auras_players(DWORD id);
void __stdcall update_aura();