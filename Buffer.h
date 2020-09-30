#pragma once
class CBuffer
{
public:
	CBuffer(void);
	~CBuffer(void);
private:
	UCHAR* m_buffer;
	ULONG m_length;
	ULONG m_buffer_length;
public:
	void Append(UCHAR* buffer,ULONG length);
	void Draw(ULONG count);
	UCHAR* GetBuffer(void);
	ULONG GetLength(void);
	void Clear(void);
};

