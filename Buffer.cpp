#include "main.h"
#include "Buffer.h"


CBuffer::CBuffer(void)
	: m_buffer(NULL)
	, m_length(0)
	, m_buffer_length(0)
{
	m_buffer = new UCHAR[4096];
	m_buffer_length = 4096;
}


CBuffer::~CBuffer(void)
{
	if(m_buffer)
		delete m_buffer;
}


void CBuffer::Append(UCHAR* buffer,ULONG length)
{
	if(!m_buffer)//Пустой буффер значит что там ничего нет
	{
		m_buffer = new UCHAR[length];
		m_buffer_length = length;
	}

	if(m_buffer_length-m_length<length)//Нехватает места(
	{
		UCHAR* newbuf = new UCHAR[m_length+length];
		memcpy(newbuf,m_buffer,m_length);
		delete m_buffer;
		m_buffer = newbuf;
		m_buffer_length = m_length+length;
	}
	memcpy(m_buffer+m_length,buffer,length);
	m_length += length;
}


void CBuffer::Draw(ULONG count)
{
	count = min(count,m_length);
	ULONG newLen = m_length-count;
	if(newLen)
		for(ULONG i=0;i<newLen;i++)
		{
			m_buffer[i] = m_buffer[i+count];
		}
	m_length = newLen;
}


UCHAR* CBuffer::GetBuffer(void)
{
	return m_buffer;
}


ULONG CBuffer::GetLength(void)
{
	return m_length;
}


void CBuffer::Clear(void)
{
	if(m_buffer)
		delete m_buffer;
	m_length = 0;
	m_buffer = new UCHAR[4096];
	m_buffer_length = 4096;
}
