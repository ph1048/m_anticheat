#include "features.h"
#include <map>

using namespace std;

unsigned long mapinfo_flags = 0;

#define DELETE_PACKET 1
#define PASS_PACKET 0

struct name_color
{
	UINT32 fg;
	UINT32 bg;
};

struct aura
{
	UINT8 aura_f;
	UINT8 aura_b;
	UINT8 aura_a;
};

struct aura my_aura = {0};

map<UINT32,aura> players_auras;
map<UINT32,name_color> name_colors;

int parse_packet(UCHAR* buf,UINT32 len,int* remove)
{
	if(*(UINT16*)buf == 0x1339 && len == 6)
	{
		mapinfo_flags = *(unsigned long*)(buf+2);		
		return DELETE_PACKET;
	}

	if(*(UINT16*)buf == 0x856 || *(UINT16*)buf == 0x857 || *(UINT16*)buf == 0x858)
	{
		struct aura _tmpaura = {0};
		_tmpaura.aura_f = 0xff&*(UINT8*)(buf+len-3);
		_tmpaura.aura_b = 0xff&*(UINT8*)(buf+len-2);
		_tmpaura.aura_a = 0xff&*(UINT8*)(buf+len-1);

		players_auras[*(UINT32*)(buf+5)] = _tmpaura;
		
		return PASS_PACKET;
	}
	
	if(*(UINT16*)buf == 0x1338 && len==14)
	{
		name_color color;
		color.fg = *(UINT32*)(buf+6);
		color.bg = *(UINT32*)(buf+10);
		name_colors[*(UINT32*)(buf+2)] = color;
		return DELETE_PACKET;
	}
	if(*(UINT16*)buf == 0x80 && len==7) //clif_clearunit_single
	{
		name_colors.erase(*(UINT32*)(buf+2));
		players_auras.erase(*(UINT32*)(buf+2));
		return PASS_PACKET;
	}
	if(*(UINT16*)buf == 0x81 && len==3) //authfail
	{
		memset(&my_aura,'\0',sizeof(my_aura));
		name_colors.clear();
		players_auras.clear();
		dwMouseState = 0;
		return PASS_PACKET;
	}
	if(*(UINT16*)buf == 0xB3 && len==6) //char select ok
	{
		*remove = 3;
		my_aura.aura_f = 0xff&*(UINT8*)(buf+len-3);
		my_aura.aura_b = 0xff&*(UINT8*)(buf+len-2);
		my_aura.aura_a = 0xff&*(UINT8*)(buf+len-1);
		name_colors.clear();
		players_auras.clear();
		dwMouseState = 0;
		return PASS_PACKET;
	}
	if(*(UINT16*)buf == 0x73 && len==14) //authok
	{
		*remove = 3;
		my_aura.aura_f = 0xff&*(UINT8*)(buf+len-3);
		my_aura.aura_b = 0xff&*(UINT8*)(buf+len-2);
		my_aura.aura_a = 0xff&*(UINT8*)(buf+len-1);
		name_colors.clear();
		dwMouseState = 0;
		players_auras.clear();
		return PASS_PACKET;
	}
	if(*(UINT16*)buf == 0x91 && len==25) //clif_changemap
	{
		*remove = 3;
		my_aura.aura_f = 0xff&*(UINT8*)(buf+len-3);
		my_aura.aura_b = 0xff&*(UINT8*)(buf+len-2);
		my_aura.aura_a = 0xff&*(UINT8*)(buf+len-1);
		name_colors.clear();
		players_auras.clear();
		dwMouseState = 0;
		return PASS_PACKET;
	}
	return PASS_PACKET;
}

int get_id_name_color(UINT32 id,UINT32* fg,UINT32* bg)
{
	map<UINT32,name_color>::iterator it;
	it=name_colors.find(id);
	if(it == name_colors.end())
		return 0;
	*fg = (*it).second.fg;
	*bg = (*it).second.bg;
	return 1;
}

void __stdcall update_auras_players(DWORD id)
{
	__try
	{
		id = *(PDWORD)(id+0x108);
		id = *(PDWORD)(id+0x224);
		if(*(PDWORD)0x8BC8C8 == id)
		{
			update_aura();
			return;
		}
		UINT8 f,b,a;
		map<UINT32,aura>::iterator it;
		it=players_auras.find((UINT32)id);
		if(it == players_auras.end())
			f=b=a=0;
		else
		{
			f=(*it).second.aura_f;
			b=(*it).second.aura_b;
			a=(*it).second.aura_a;
		}

		wsprintf((char*)0x8F80A0,"aura\\f%.03d.bmp",f);
		wsprintf((char*)0x8F80B0,"aura\\b%.03d.bmp",b);
		wsprintf((char*)0x8F80C0,"aura\\a%.03d.bmp",a);
	}
	__except(1)
	{
	}
}

void __stdcall update_aura()
{
	wsprintf((char*)0x8F80A0,"aura\\f%.03d.bmp",my_aura.aura_f);
	wsprintf((char*)0x8F80B0,"aura\\b%.03d.bmp",my_aura.aura_b);
	wsprintf((char*)0x8F80C0,"aura\\a%.03d.bmp",my_aura.aura_a);
}