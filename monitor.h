#include "main.h"


__declspec(align(1)) struct EDID
{
	BYTE Header[8];
	WORD ManufacturerID;
	WORD ProductID;
	DWORD SerialNumber;
	BYTE WeekOfManufacture;
	BYTE YearOfManufacture;
	WORD EDIDVersion;
	BYTE BasicDisplayParameters[5];
	BYTE ChromaticityCoordinates[10];
	BYTE Timings[3];
	BYTE StandardTimingIdentification[16];
	BYTE Db1[18];
	BYTE Db2[18];
	BYTE Db3[18];
	BYTE Db4[18];
	BYTE ExtensionFlag;
	BYTE Checksum;
};

int GetMonitorSN(char* serial);