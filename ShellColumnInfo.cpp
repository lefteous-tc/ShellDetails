#include "ShellColumnInfo.h"

#include "contentplug.h"
// #include <Rpc.h>
// #include <Rpcdce.h>

/*
#include <fstream>
using namespace std;

ofstream o2 ("C:\\Log\\sd_sci.log");
*/

ShellColumnInfo::ShellColumnInfo ():
m_pIniFile(NULL),
m_useDefault (TRUE),
m_processing(0),
m_columnState(0)
{
}

ShellColumnInfo::ShellColumnInfo (BasicINIFile* iniFilePath):
m_pIniFile(iniFilePath),
m_useDefault (TRUE),
m_processing(0),
m_columnState(0)
{
}


ShellColumnInfo::ShellColumnInfo (BasicINIFile* iniFilePath, SHCOLUMNID columnID):
m_pIniFile(iniFilePath),
m_columnID(columnID),
m_useDefault (TRUE),
m_processing(0),
m_columnState(0)
{
}

void ShellColumnInfo::load()
{
	// GUID extraction
	TCHAR* section = m_pIniFile->getsection();	
	TCHAR* guid1 = _tcschr (section, '{');
	TCHAR* guid2 = _tcschr (section, '}');
	TCHAR guid[MAX_PATH] = {0};
	// Cut the GUID, we don't need the brackets for UuidFromString.	
	StringCchCopy (guid, _tcslen (guid1) - _tcslen (guid2), section+1);

#ifdef UNICODE
	UuidFromString ((RPC_WSTR)(TCHAR*)guid, &m_columnID.fmtid);
#else
	UuidFromString ((unsigned char*)guid, &m_columnID.fmtid);
#endif

	
	// id extraction
	TCHAR* id = _tcschr (section, '}');
	id++;
	m_columnID.pid = _tstoi (id);

	// Read other settings.
	m_useDefault = m_pIniFile->readInteger (TEXT("UseDefault"), 1);
	m_processing =  m_pIniFile->readInteger (TEXT("Processing"), 0);
	m_cast = m_pIniFile->readInteger (TEXT("Cast"), ft_string);
	m_calcOperator = (TCHAR)m_pIniFile->readInteger (TEXT("Operator"), '/');
	m_calcOperand = m_pIniFile->readInteger (TEXT("Operand"), 1);
	m_columnState = m_pIniFile->readInteger (TEXT("State"), 0);

	StringCchCopy (m_columnTitle,_MAX_PATH, m_pIniFile->readString (TEXT("Caption"), TEXT("")));
	filterFieldName (m_columnTitle);	
}

ShellColumnInfo::~ShellColumnInfo()
{
}

void ShellColumnInfo::save(BOOL asDefault)
{
	m_pIniFile->writeString (TEXT("Caption"), m_columnTitle);
	m_pIniFile->writeInteger (TEXT("UseDefault"), asDefault);	
	m_pIniFile->writeInteger (TEXT("State"), m_columnState);
}

BOOL ShellColumnInfo::getUseDefault()
{
	return m_useDefault;
}

int ShellColumnInfo::getColumnType() const
{
	int type = ft_string;
	if (!m_useDefault)
	{
		type = m_cast;		
	}
	else
	{		
		type = m_columnState;
		type &= 3;		

		// Masking anything out that is not a necessary to detect the field type.		
		if (type == SHCOLSTATE_TYPE_STR)
		{
			type = ft_string;
		}
		else if (type == SHCOLSTATE_TYPE_INT)
		{
			type = ft_numeric_64;
		}
		else if (type == SHCOLSTATE_TYPE_DATE)
		{
			type = ft_datetime;
		}
		else
		{
			type = ft_string;
		}
	}	
	return type;
}

void ShellColumnInfo::setColumnID (SHCOLUMNID columnID)
{
	m_columnID = columnID;
}

SHCOLUMNID& ShellColumnInfo::getColumnID()
{
	return m_columnID;
}

void ShellColumnInfo::setColumnTitle (TCHAR* columnTitle)
{
	StringCchCopy (m_columnTitle, MAX_PATH, columnTitle);
	filterFieldName (m_columnTitle);
}

TCHAR* ShellColumnInfo::getColumnTitle()
{
	return m_columnTitle;
}

void ShellColumnInfo::setColumnState (SHCOLSTATEF columnState)
{
	m_columnState = columnState;
}

SHCOLSTATEF ShellColumnInfo::getColumnState()
{
	return m_columnState;
}

int ShellColumnInfo::getCast()
{
	return m_cast;
}

TCHAR ShellColumnInfo::getCalcOperator()
{
	return m_calcOperator;
}

int ShellColumnInfo::getCalcOperand()
{
	return m_calcOperand;
}

int ShellColumnInfo::getProcessing()
{
	return m_processing;
}

void ShellColumnInfo::filterFieldName(TCHAR* fieldName)
{
	size_t fieldNameLength = _tcslen (fieldName);
	// Avoid . : | characters in field names.
	for (size_t pos = 0; pos < fieldNameLength; pos++)
	{
		TCHAR curChar = fieldName[pos];
		if (curChar == '.' || curChar == ':' || curChar == '|')
		{
			fieldName[pos] = '_';
		}
		else if (curChar == '[' )
		{
			fieldName[pos] = '{';
		}
		else if (curChar == ']' )
		{
			fieldName[pos] = '}';
		}
	}
}

bool ShellColumnInfo::existsCurrentColumnID (SHCOLUMNID& columnID)
{
	// Create string section from SHCOLUMNID.
	TCHAR pid [MAX_PATH] = {0};
	_itow_s (columnID.pid, pid, 10);	
	wchar_t guidW[MAX_PATH] = {0};	
	TCHAR section[MAX_PATH] = {0};
	StringFromGUID2 (columnID.fmtid, guidW, MAX_PATH);
#ifdef UNICODE
	StringCchCat (section, MAX_PATH, guidW);
#else
	TCHAR guid[MAX_PATH] = {0};
	WideCharToMultiByte (CP_ACP, 0, guidW, -1, guid, sizeof (guid), NULL, NULL);
	StringCchCat (section, MAX_PATH, guid);
#endif	
	
	StringCchCat (section, MAX_PATH, pid);
	m_pIniFile->setSection (section);

	int sectionExists = m_pIniFile->readInteger (TEXT("UseDefault"), -1);
	return sectionExists != -1;
}