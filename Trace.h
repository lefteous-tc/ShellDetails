#pragma once

#include <windows.h>
#include <strsafe.h>

class Trace
{
public:
	Trace(char* path);
	~Trace(void);	
	void write(char* text);
	void writeNumber (int number);
	void writeLargeNumber (__int64 number);
	void writeBreak();
protected:
	HANDLE m_Handle;
};
