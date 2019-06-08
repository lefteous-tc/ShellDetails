#include "Trace.h"

Trace::Trace(char* path)
{
	m_Handle = CreateFile (path, GENERIC_WRITE, FILE_SHARE_READ, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 	
}

Trace::~Trace(void)
{
	CloseHandle (m_Handle);
}

void Trace::write(char* text)
{		
	DWORD fileWritten = 0;
	WriteFile (m_Handle, text, strlen(text), &fileWritten, NULL);	
}

void Trace::writeBreak()
{	
	DWORD fileWritten = 0;
	WriteFile (m_Handle, "\r\n", 2, &fileWritten, NULL);
}

void Trace::writeNumber(int number)
{	
	DWORD fileWritten = 0;
	char c[65];
	itoa (number, c, 10);
	WriteFile (m_Handle, c, strlen(c), &fileWritten, NULL);
}

void Trace::writeLargeNumber (__int64 number)
{
	DWORD fileWritten = 0;
	char c[65];
	_i64toa (number, c, 10);
	WriteFile (m_Handle, c, strlen(c), &fileWritten, NULL);
}