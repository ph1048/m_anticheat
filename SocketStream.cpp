#include "main.h"
#include "Buffer.h"
#include "SocketStream.h"
#include "features.h"


CSocketStream::CSocketStream(void)
{
	m_rawdata.Clear();
	m_socketdata.Clear();
}


CSocketStream::~CSocketStream(void)
{
}


void CSocketStream::RecvFromServer(UCHAR* buf, ULONG len)
{
	m_rawdata.Append(buf,len);
	while(m_rawdata.GetBuffer() && *(UINT32*)m_rawdata.GetBuffer()+4 <= m_rawdata.GetLength())
	{
		int toremove = 0;
		PosDecryptNet(m_rawdata.GetBuffer()+4,*(UINT32*)m_rawdata.GetBuffer());
#ifdef SERVER_TWINKLE
		if(!parse_packet(m_rawdata.GetBuffer()+4,*(UINT32*)m_rawdata.GetBuffer(),&toremove))
#endif
		{
			m_socketdata.Append(m_rawdata.GetBuffer()+4,*(UINT32*)m_rawdata.GetBuffer()-toremove);
		}
		m_rawdata.Draw(*(UINT32*)m_rawdata.GetBuffer()+4);
	}
}


bool CSocketStream::HasSomethingToRead(void)
{
	return(m_socketdata.GetLength()>0);
}


void CSocketStream::Reset(void)
{
	m_rawdata.Clear();
	m_socketdata.Clear();
}


ULONG CSocketStream::RecvToClient(UCHAR* buf, ULONG len)
{
	if(!HasSomethingToRead())
		return -1;
	
	ULONG ulToRead = min(len,m_socketdata.GetLength());
	memcpy(buf,m_socketdata.GetBuffer(),ulToRead);
	m_socketdata.Draw(ulToRead);
	return ulToRead;
}
