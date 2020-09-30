#include <windows.h>
#include <nt/ntdll.h>

void PosEncrypt(unsigned char* source, int len, unsigned char* keypos);
void PosDecrypt(unsigned char* source, int len, unsigned char* keypos);
void PosEncryptNet(UCHAR* source, UINT32 len, UCHAR* keypos);
void PosDecryptNet(UCHAR* source, UINT32 len);
/*
unsigned char BitTwist(unsigned char in);
unsigned char BitUntwist(unsigned char in);*/