#pragma once
#include "buffer.h"
#include <deque>

class CSocketStream
{
public:
	CSocketStream(void);
	~CSocketStream(void);
	void RecvFromServer(UCHAR* buf, ULONG len);
	bool HasSomethingToRead(void);
	void Reset(void);
	ULONG RecvToClient(UCHAR* buf, ULONG len);
private:
	CBuffer m_rawdata;
	CBuffer m_socketdata;
};

