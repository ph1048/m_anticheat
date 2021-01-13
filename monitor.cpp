#include "monitor.h"
#include "strcrypt.h"

int CheckSum(BYTE* edid)
{
	BYTE sum = 0;
	for(int i=0;i<128;i++)
		sum += edid[i];
	return sum==0;
}

int GetMonitorSN(char* serial)
{
	HKEY hDisplays;
	RegOpenKeyExA(HKEY_LOCAL_MACHINE,F("SYSTEM\\CurrentControlSet\\Enum\\DISPLAY"),NULL,KEY_READ,&hDisplays);
	char szDisplayName[256] = {0};
	char szConfigName[256] = {0};

	BYTE edid[256] = {0};
	DWORD edidsize = sizeof(edid);

	DWORD size = 255;

	for(DWORD i=0;RegEnumKeyExA(hDisplays,i,szDisplayName,&size,NULL,NULL,NULL,NULL)!=ERROR_NO_MORE_ITEMS;i++)
	{//����������� �������
		HKEY hDisplayKey;
		RegOpenKeyExA(hDisplays,szDisplayName,NULL,KEY_READ,&hDisplayKey);
		size = 255;
		for(DWORD j=0;RegEnumKeyExA(hDisplayKey,j,szConfigName,&size,NULL,NULL,NULL,NULL)!=ERROR_NO_MORE_ITEMS;j++)
		{//����������� �������
			HKEY hConfigKey,hParamsKey,hControlKey;
			size = 255;
			if(RegOpenKeyExA(hDisplayKey,szConfigName,NULL,KEY_READ,&hConfigKey)!=ERROR_SUCCESS)
				continue;
			if(RegOpenKeyExA(hConfigKey,F("Device Parameters"),NULL,KEY_READ,&hParamsKey)!=ERROR_SUCCESS)
				continue;
			//if(RegOpenKeyExA(hConfigKey,F("Control"),NULL,KEY_READ,&hControlKey)==ERROR_FILE_NOT_FOUND)
			//	continue;
			if(RegQueryValueExA(hParamsKey,F("EDID"),NULL,NULL,(LPBYTE)edid,&edidsize)==ERROR_SUCCESS)
			{
				if(*(PDWORD)edid!=0xffffff00)
					continue;//�� �������� ���������

				char* s;
				if(((EDID*)edid)->Db4[3]==0xFF)//���� �������� � description ������
					s = (char*)&((EDID*)edid)->Db4[5];
				else if(((EDID*)edid)->Db3[3]==0xFF)
					s = (char*)&((EDID*)edid)->Db3[5];
				else if(((EDID*)edid)->Db2[3]==0xFF)
					s = (char*)&((EDID*)edid)->Db2[5];
				else if(((EDID*)edid)->Db1[3]==0xFF)
					s = (char*)&((EDID*)edid)->Db1[5];
				/*else if(strlen((char*)&((EDID*)edid)->Db4[5])>0)//�� ����� - �������� ����� �� 4�� ���� ��� ���� �����
					s = (char*)&((EDID*)edid)->Db4[5];
				else if(strlen((char*)&((EDID*)edid)->Db3[5])>0)//�� ����� - �������� ����� �� 3�� ���� ��� ���� �����
					s = (char*)&((EDID*)edid)->Db3[5];
				else if(strlen((char*)&((EDID*)edid)->Db2[5])>0)//�� ����� - �������� ����� �� 2�� ���� ��� ���� �����
					s = (char*)&((EDID*)edid)->Db2[5];
				else if(strlen((char*)&((EDID*)edid)->Db1[5])>0)//�� ����� - �������� ����� �� 1�� ���� ��� ���� �����
					s = (char*)&((EDID*)edid)->Db1[5];*/
				else
					continue;//�� ��� ��� ���.



				if(strtrim(serial,s,13)>0)
				{
					if(CheckSum(edid))
						strcat(serial,F("_OK"));
					else
						strcat(serial,F("_NO"));
					return 1;
				}
			}
			size = 255;
		}
		size = 255;
	}
	ZeroMemory(serial,13);
	return 0;
}