#include "network.h"
#include "strcrypt.h"

int GetRouterMACaddress(BYTE* mac)
{
	DWORD bi = 0;
	
	GetBestInterface(inet_addr(F("8.8.8.8")),&bi);

	MIB_IPNETTABLE n[32] = {0};
	DWORD s = sizeof(n);

	GetIpNetTable(n,&s,0);

	if(n->dwNumEntries<1)
		return 0;

	for(int i=0;i<n->dwNumEntries;i++)
	{
		if(n->table[i].dwIndex == bi)
		{
			if(n->table[i].dwPhysAddrLen<6)
				continue;

			memcpy((void*)mac,(void*)n->table[i].bPhysAddr,n->table[i].dwPhysAddrLen);
			return 1;
		}
	}

	return 0;
}

int GetMACaddress(BYTE* mac)
{
	DWORD bi = 0;
	
	GetBestInterface(inet_addr(F("8.8.8.8")),&bi);

	IP_ADAPTER_INFO n[32] = {0};
	DWORD s = sizeof(n);

	GetAdaptersInfo(n,&s);

	for(PIP_ADAPTER_INFO p = n; p!=0; p = p->Next)
	{
		if(p->Index == bi)
		{
			if(p->AddressLength<6)
				continue;

			memcpy((void*)mac,(void*)p->Address,p->AddressLength);
			return 1;
		}
	}

	return 0;
}