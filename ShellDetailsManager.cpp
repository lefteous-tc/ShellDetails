#include "ShellDetailsManager.h"

#include "contentplug.h"
#include "PathUtils.h"


#include <float.h>

ShellDetailsManager::ShellDetailsManager():
m_pCurrentShellFolder(NULL),
m_pDesktop(NULL),
m_pidl(NULL),
m_untitledCount(0),
m_sectionCount(0),
//m_pMalloc(NULL),
m_lastpidl(NULL),
m_logger(L"ShellDetails.wdx", L"ShellDetailsManager", L"C:\\Log\\ShellDetails.log")
{	
	CoInitialize(NULL);	
	//SHGetMalloc (&m_pMalloc);	
	/*
	ZeroMemory (m_currentDirectoryPath, sizeof (TCHAR) * MAX_PATH);
	ZeroMemory (m_currentPath, sizeof (TCHAR) * MAX_PATH);
	ZeroMemory (m_iniFilePath, sizeof (TCHAR) * MAX_PATH);
	*/
}

ShellDetailsManager::~ShellDetailsManager()
{	
	if (m_pDesktop)
	{
		m_pDesktop->Release();				
		m_pDesktop = NULL;
	}
	if (m_pCurrentShellFolder)
	{		
		m_pCurrentShellFolder->Release();
		m_pCurrentShellFolder = NULL;
	}
	if (m_pidl)
	{
		//m_pMalloc->Free(m_pidl);
		ILFree (m_pidl);
	}	
	CoUninitialize();
	if (m_pShellColumnInfo)
	{
		delete [] m_pShellColumnInfo;
	}
}

ShellColumnInfo* ShellDetailsManager::getAvailableFields()
{
	return m_pShellColumnInfo;
}
//--------------------------------------------------------------------------
int ShellDetailsManager::extractValue (const int fieldIndex, void* fieldValue)
{		
	HRESULT r = S_OK;
	int	result = ft_fieldempty;
	// Get all required information for GetDetailsEx and call it.
	ShellColumnInfo ci = m_pShellColumnInfo[fieldIndex];
	VARIANT columnValue;
	ZeroMemory (&columnValue, sizeof(VARIANT));	
	if (m_pCurrentShellFolder && m_lastpidl)
	{
		IShellFolder2* pShellFolder2 = (IShellFolder2*)m_pCurrentShellFolder;
		unsigned int control_word = 0;
		_controlfp_s(&control_word, 0, 0);		
		unsigned int oldControlWord = control_word;
		_controlfp_s(&control_word, _MCW_EM, control_word | _EM_INVALID | _EM_ZERODIVIDE);		
		r = pShellFolder2->GetDetailsEx (m_lastpidl, &ci.getColumnID(), &columnValue);		
		/*m_logger.log (extractValue, r);*/
		_controlfp_s(&control_word, _MCW_EM, oldControlWord);		
		if (r != S_OK)
		{
			return ft_fieldempty;
		}
	}
	else
	{		
		return ft_fieldempty;
	}	

	// Extract the field values from the variant variable.	
	/*m_logger.log (L"Field type", columnValue.vt);*/
	switch (columnValue.vt)
	{
	case VT_ARRAY | VT_BSTR:
		result = extractArrayStringValue (fieldValue, columnValue, ci);
		break;
	case VT_BSTR:	
		result = extractStringValue (fieldValue, columnValue, ci);				
		break;
	case VT_I1:
	case VT_UI1:
	case VT_UI2:
	case VT_UI4:
	case VT_I4:
	case VT_I8:
	case VT_UI8:
	case VT_INT:
	case VT_UINT:	
		result = extractNumericValue (fieldValue, columnValue, ci);
		break;	
	case VT_DATE:
		result = extractDateValue (fieldValue, columnValue);
		break;
	case VT_BOOL:		
		*(BOOL*)fieldValue = columnValue.boolVal;
		result = ft_boolean;
		break;
	case VT_NULL: 
	case VT_EMPTY: 
	case VT_ERROR:
		result = ft_fieldempty;
		break;
	default:
		result = ft_fieldempty;
		break;
	}
	//m_logger.log (L"extractValue", result);
	return result;
}
//--------------------------------------------------------------------------
void ShellDetailsManager::init ()
{	
	//BOOL readFromShell = FALSE;
	// Initialize the desktop folder (once and for all)
	SHGetDesktopFolder(&m_pDesktop);


	/*
	
	//IShellFolder2* pMyDesktop2 = (IShellFolder2*)m_pDesktop;
	//IEnumExtraSearch *pEnum = NULL;
	IEnumIDList* penumIDList = NULL;
	m_pDesktop->EnumObjects (NULL, SHCONTF_FOLDERS, &penumIDList);
	LPITEMIDLIST pidl = NULL;
	ULONG fetched = 0;
	HRESULT res = penumIDList->Next (1, &pidl, &fetched);
	while (res != S_FALSE)
	{

	}
	*/
	
	/*
	if (r != S_OK)
	{
		return;
	}
	
	if (readFromShell)
	{
		
	}	
	*/
	getColumnsFromShell();
	getColumnsFromINIFile();
}
//--------------------------------------------------------------------------
int ShellDetailsManager::initDirectory (TCHAR* directory)
{
	// o << "init directory: " << directory << endl;
	if (!m_pDesktop)
	{
		// o << "init directory failed (1): " << directory << endl;
		return ft_nomorefields;
	}
	IShellFolder2*	pCurrentShellFolder = NULL;
	/*
	if ( bindDirectory (directory, m_pDesktop, pCurrentShellFolder) == ft_fieldempty)
	{
		return ft_fieldempty;
	}
	*/
	LPITEMIDLIST	pidlCurrentDir	= NULL;		

#ifdef UNICODE
	HRESULT r = m_pDesktop->ParseDisplayName (NULL, NULL, directory, NULL, &pidlCurrentDir, NULL);	
#else
	wchar_t pathW[MAX_PATH] = {0};	
	MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, directory, -1, pathW, MAX_PATH);		
	HRESULT r = m_pDesktop->ParseDisplayName (NULL, NULL, pathW, NULL, &pidlCurrentDir, NULL);	
#endif

	
	if (r != S_OK)
	{		
		// o << "init directory failed (2): " << directory << endl;
		return ft_nomorefields;
	}
	r = m_pDesktop->BindToObject(pidlCurrentDir, NULL, IID_IShellFolder2, (LPVOID *)&pCurrentShellFolder);
	if (r != S_OK)
	{		
		// o << "init directory failed (3): " << directory << endl;
		return ft_nomorefields;
	}

	if (pidlCurrentDir)
	{
		//m_pMalloc->Free(m_pidl);
		ILFree (pidlCurrentDir);		
	}	
	
	SHELLDETAILS shellDetails;
	ZeroMemory (&shellDetails, sizeof (SHELLDETAILS));
	int iSubItem = 0;
	
	SectionManagerINIFile iniFile (m_iniFilePath, TEXT(""));	

	// We'are asking the object for the list of available columns.
	// For each, we are adding them to the control in the right order.
	r = pCurrentShellFolder->GetDetailsOf (NULL , iSubItem, &shellDetails);	
	// o << "enum fields start: " << (long)r << endl;
	while (r == S_OK)
	{			
		// Map current column ID to SHCOLUMNID structure
		SHCOLUMNID columnID;
		pCurrentShellFolder->MapColumnToSCID (iSubItem, &columnID);
		// Is this value already stored in INI file
		ShellColumnInfo ci (&iniFile, columnID);
		// The column state must be always retrieved and set. It's not stored in the INI file!
		SHCOLSTATEF columnState = 0;
		pCurrentShellFolder->GetDefaultColumnState (iSubItem, &columnState);
		ci.setColumnState (columnState);
		TCHAR fieldName[MAX_PATH];
		// If this column is a new one not found in the INI file.
		if (!ci.existsCurrentColumnID (columnID))
		{
			
			
#ifdef UNICODE
			StringCchCopy (fieldName, MAX_PATH, shellDetails.str.pOleStr);			
#else
			WideCharToMultiByte (CP_ACP, 0, shellDetails.str.pOleStr, -1, fieldName, MAX_PATH, NULL, NULL);			
#endif
			
			// Is this a named entry?
			if (_tcslen(fieldName))
			{
				// Write the name provided by the shell to the INI file.
				ci.setColumnTitle (fieldName);
				// o << "enum fields: new fields added: " << fieldName << endl;
			}
			else
			{
				// Write a generic name to the INI file.
				TCHAR untitled [MAX_PATH] = {0};
				TCHAR untitledCountNr [MAX_PATH] = {0};
				StringCchCopy (untitled, MAX_PATH, TEXT("Untitled "));
				_itot_s (m_untitledCount, untitledCountNr, 10);
				StringCchCat (untitled, MAX_PATH, untitledCountNr);
				ci.setColumnTitle (untitled);
				m_untitledCount++;
				// o << "enum fields: new fields added: " << untitled << endl;
			}
			ci.save(ci.getUseDefault());			
		} // end !currentColumnExists
		/*
		else
		{
			o << "enum fields: field already exists" << endl;
		}
		*/
		iSubItem++;
		r = pCurrentShellFolder->GetDetailsOf (NULL, iSubItem, &shellDetails);
		// o << "enum fields next: " << (long)r << endl;
	} // end while
	if (pCurrentShellFolder)
	{
		pCurrentShellFolder->Release();
		pCurrentShellFolder = NULL;
	}
	// o << "enum fields complete" << endl;
	return 0;
}
//--------------------------------------------------------------------------
void ShellDetailsManager::setINIFilePath (TCHAR* iniFilePath)
{
	StringCchCopy (m_iniFilePath, MAX_PATH, iniFilePath);
}
//--------------------------------------------------------------------------
TCHAR* ShellDetailsManager::getINIFilePath ()
{
	return m_iniFilePath;
}
//--------------------------------------------------------------------------
// POST: m_pCurrentShellFolder will contain a valid  object of type IShellFolder2 which points to the current directory.
int ShellDetailsManager::bindCurrentDirectory (TCHAR* currentDirectoryPath)
{
	if (m_pCurrentShellFolder)
	{		
		m_pCurrentShellFolder->Release ();
		m_pCurrentShellFolder = NULL;
	}

	LPITEMIDLIST pidlCurrentDir	= NULL;

#ifdef UNICODE
	HRESULT r = m_pDesktop->ParseDisplayName (NULL, NULL, currentDirectoryPath, NULL, &pidlCurrentDir, NULL);
#else
	wchar_t currentDirectoryPathW[MAX_PATH] = {0};
	MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, currentDirectoryPath, -1, currentDirectoryPathW, MAX_PATH);
	HRESULT r = m_pDesktop->ParseDisplayName (NULL, NULL, currentDirectoryPathW, NULL, &pidlCurrentDir, NULL);
#endif
	
	if (r != S_OK)
	{		
		return ft_fieldempty;
	}

	r = m_pDesktop->BindToObject(pidlCurrentDir, NULL, IID_IShellFolder2, (LPVOID *)&m_pCurrentShellFolder);			
	if (pidlCurrentDir)
	{		
		//m_pMalloc->Free(m_pidl);
		ILFree (pidlCurrentDir);
	}
	if (r != S_OK)
	{		
		return ft_fieldempty;
	}
	return 0;
}
//--------------------------------------------------------------------------
int ShellDetailsManager::getAvailableFieldsCount()
{
	return m_sectionCount;
}
//--------------------------------------------------------------------------
void ShellDetailsManager::setExtractionDirectory (const TCHAR* path)
{	
	const TCHAR* currentDirectory = _tcsrchr (path, '\\');
	TCHAR currentDirectoryPath[MAX_PATH] = {0};
	StringCchCopyN (currentDirectoryPath, MAX_PATH,  path, _tcslen(path) - _tcslen(currentDirectory) +1);

	if (_tcsicmp(currentDirectoryPath, m_currentDirectoryPath) != 0)
	{			
		bindCurrentDirectory(currentDirectoryPath);
		StringCchCopy (m_currentDirectoryPath, MAX_PATH, currentDirectoryPath);			
	}
}
//--------------------------------------------------------------------------
void ShellDetailsManager::setExtractionItemIDLIst (const TCHAR* path)
{
	UNIQUE_FILE_IDENTIFIER fid = PathUtils::getUniqueFileIdentifier (path);
	/*m_logger.log (L"---Start---", fid.fileIndex.QuadPart);	*/
	//m_logger.log (path, fid.fileIndex.QuadPart);	
	// Do we really need to retrieve a pidl (filename is different)?	
	if (_tcsicmp(path, m_currentPath) != 0)
	{
		StringCchCopy (m_currentPath, MAX_PATH, path);		

		// Delete the old pidl if exists.
		if (m_pidl)
		{	
			//m_pMalloc->Free(m_pidl);
			ILFree (m_pidl);
			m_pidl = NULL;
		}

#ifdef UNICODE
		// Convert the path into a pidl.
		m_pDesktop->ParseDisplayName (NULL, NULL, m_currentPath, NULL, &m_pidl, NULL);
#else
		wchar_t currentPathW [MAX_PATH] = {0};		
		MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, m_currentPath, -1, currentPathW, MAX_PATH);
		// Convert the path into a pidl.
		m_pDesktop->ParseDisplayName (NULL, NULL, currentPathW, NULL, &m_pidl, NULL);
#endif

		//m_pidl = ILCreateFromPath (path);		
		m_lastpidl = ILFindLastID(m_pidl);
	}
}
//--------------------------------------------------------------------------
int ShellDetailsManager::extractStringValue (void* fieldValue, const VARIANT& columnValue, ShellColumnInfo& columnInfo)
{
	int result = ft_fieldempty;
	if (!columnInfo.getUseDefault())
	{
		if (columnInfo.getCast() == ft_numeric_64)
		{
			__int64 value = _wtoi64 (columnValue.bstrVal);
			*(__int64*)fieldValue = value;
			result = ft_numeric_64;
		}
	}
	else
	{
#ifdef UNICODE
		StringCchCopy ((TCHAR*)fieldValue, MAX_PATH, columnValue.bstrVal);
		result = ft_stringw;
#else
		WideCharToMultiByte (CP_ACP, 0, columnValue.bstrVal, -1, (TCHAR*)fieldValue, MAX_PATH, NULL, NULL);
		result = ft_string;
#endif
	}
	SysFreeString (columnValue.bstrVal);
	return result;
}
//--------------------------------------------------------------------------
int ShellDetailsManager::extractArrayStringValue (void* fieldValue, const VARIANT& columnValue, ShellColumnInfo& columnInfo)
{
	int result = ft_fieldempty;
	BSTR stringValue;
	WCHAR s [MAX_PATH] = {0};
	StringCchCopy ((TCHAR*)fieldValue, MAX_PATH, TEXT(""));

	LONG indices = 0;
	while (indices < columnValue.parray->rgsabound->cElements)
	{
		HRESULT r = SafeArrayGetElement (columnValue.parray, &indices, &stringValue);			
		if (r == S_OK)
		{				
			if (indices > 0)
			{
				StringCchCatW (s, MAX_PATH, L"; ");				
			}
			StringCchCatW (s, MAX_PATH, stringValue);
			SysFreeString (stringValue);
		}
		else
		{
			result = ft_fileerror;
		}
		++indices;
	}
	

#ifdef UNICODE	
	StringCchCopyW ((WCHAR*)fieldValue, MAX_PATH, s);
	result = ft_stringw;

#else
	WideCharToMultiByte (CP_ACP, 0, s, -1, (TCHAR*)fieldValue, MAX_PATH, NULL, NULL);		
	result = ft_string;
#endif
	
	return result;
}
//--------------------------------------------------------------------------
int ShellDetailsManager::extractNumericValue (void* fieldValue, const VARIANT& columnValue, ShellColumnInfo& columnInfo)
{
	int result = ft_fieldempty;
	if (!columnInfo.getUseDefault())
	{
		switch (columnInfo.getCast())
		{
		case ft_numeric_64:
			if (columnInfo.getCalcOperator() == '/' && columnInfo.getCalcOperand())			
			*(__int64*)fieldValue = columnValue.cyVal.int64 / columnInfo.getCalcOperand();
			result = ft_numeric_64;
			break;
		case ft_time:
			if (columnInfo.getCalcOperator() == '/' && columnInfo.getCalcOperand())
			{
				LARGE_INTEGER timeLargeInt = {0, 0};
				timeLargeInt.QuadPart = columnValue.cyVal.int64 / columnInfo.getCalcOperand();
				int timeInt = timeLargeInt.LowPart;
				ttimeformat time;
				time.wHour = WORD(timeInt / 3600);
				time.wMinute = WORD(timeInt % 3600 / 60);
				time.wSecond = WORD(timeInt % 3600 % 60);
				*(ptimeformat)fieldValue = time;
				result = ft_time;
			}							
			break;
		case ft_numeric_floating:
			{
				USER_DEFINED_NUMBER_FORMAT format;
				ZeroMemory (&format, sizeof(USER_DEFINED_NUMBER_FORMAT));
				format.value = (double)columnValue.cyVal.int64;				
				_i64tow_s(columnValue.cyVal.int64, format.displayedValue, MAX_PATH, 10);
				*(USER_DEFINED_NUMBER_FORMAT*)fieldValue = format;
				result = ft_numeric_floating;
			}
			break;
		case ft_date:
			{
				tdateformat dateFormat;
				dateFormat.wDay = 1;
				dateFormat.wMonth = 1;
				dateFormat.wYear = columnValue.cyVal.int64;				
				*(tdateformat*)fieldValue = dateFormat;
				result = ft_date;
			}
			break;
		}
	}
	else
	{
		*(__int64*)fieldValue = columnValue.cyVal.int64;
		result = ft_numeric_64;
	}	
	return result;
}
//--------------------------------------------------------------------------
int ShellDetailsManager::extractDateValue (void* fieldValue, const VARIANT& pcolumnValue)
{
	SYSTEMTIME systemTime;
	VariantTimeToSystemTime (pcolumnValue.date, &systemTime);
	FILETIME fileTime;
	SystemTimeToFileTime (&systemTime, &fileTime);
	*(FILETIME*)fieldValue = fileTime;
	return ft_datetime;
}
//--------------------------------------------------------------------------
/*
int ShellDetailsManager::bindDirectory (const char* directoryPath, IShellFolder* pDesktopShellFolder, IShellFolder2* pShellFolder2)
{
	wchar_t directoryW[MAX_PATH] = {0};
	MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, directoryPath, -1, directoryW, MAX_PATH);

	LPITEMIDLIST pidlCurrentDir	= NULL;
	HRESULT r = pDesktopShellFolder->ParseDisplayName (NULL, NULL, directoryW, NULL, &pidlCurrentDir, NULL);
	if (r != S_OK)
	{		
		return ft_fieldempty;
	}

	r = pDesktopShellFolder->BindToObject(pidlCurrentDir, NULL, IID_IShellFolder2, (LPVOID *)&pShellFolder2);			
	if (pidlCurrentDir)
	{		
		//m_pMalloc->Free(m_pidl);
		ILFree (pidlCurrentDir);
	}
	if (r != S_OK)
	{		
		return ft_fieldempty;
	}
	return 0;
}
*/
//--------------------------------------------------------------------------
void ShellDetailsManager::getColumnsFromShell ()
{
	// Search all directories for columns
	SectionManagerINIFile iniFile (m_iniFilePath, TEXT("Directories"));	
	TCHAR dirKeyNumber [MAX_PATH] = {0};
	TCHAR dirKeyPrefix [MAX_PATH] = {0};
	TCHAR dirKeyName [MAX_PATH] = {0};
	StringCchCopy (dirKeyPrefix, MAX_PATH, TEXT("Dir_"));
	StringCchCopy (dirKeyName, MAX_PATH, dirKeyPrefix);
	int index = 1;
	_itow_s (index, dirKeyNumber, 10);
	StringCchCat (dirKeyName, MAX_PATH, dirKeyNumber);
	TCHAR* dir = iniFile.readString (dirKeyName, TEXT(""));
	bool oneDirRead = false;	

	// Initialize all user-defined search directories.
	while (_tcslen(dir))
	{		
		oneDirRead = true;
		WIN32_FIND_DATA findData;
		ZeroMemory (&findData, sizeof (WIN32_FIND_DATA));
		TCHAR searchDir [MAX_PATH] = {0};
		StringCchCat (searchDir, MAX_PATH, dir);
		StringCchCat (searchDir, MAX_PATH, TEXT("*.*"));
		HANDLE hFind = FindFirstFile (searchDir, &findData);		
		if (hFind != INVALID_HANDLE_VALUE)
		{
			initDirectory (dir);
		}		
		FindClose(hFind);
		index++;
		_itow_s (index, dirKeyNumber, 10);
		StringCchCat (dirKeyPrefix, MAX_PATH, dirKeyNumber);
		StringCchCopy (dirKeyName, MAX_PATH, dirKeyPrefix);
		dir = iniFile.readString (dirKeyName, TEXT(""));				
	}

	// No sections found --> Use the columns found in the windows dir as default.
	if (!oneDirRead)
	{
		TCHAR windir [MAX_PATH] = {0};
		GetWindowsDirectory (windir, MAX_PATH);		
		initDirectory (windir);		
	}
}
//--------------------------------------------------------------------------
void ShellDetailsManager::getColumnsFromINIFile ()
{
	SectionManagerINIFile iniFile (m_iniFilePath, TEXT("Directories"));
	// Create list of columns from INI file.
	int sectionCount = iniFile.getSectionCount(TEXT("{"), 0);

	if (sectionCount)
	{		
		m_pShellColumnInfo = new ShellColumnInfo[sectionCount];		

		m_sectionCount = sectionCount;
		const TCHAR* pSection = iniFile.getFirstSectionName();		
		int fieldIndex = 0;
		while (pSection)
		{			
			iniFile.setSection (pSection);
			if (_tcsstr(pSection, TEXT("{")))
			{
				ShellColumnInfo shellColumnInfo (&iniFile);	
				shellColumnInfo.load();
				m_pShellColumnInfo[fieldIndex] = shellColumnInfo;					
			}
			else
			{
				fieldIndex--;
			}
			pSection = iniFile.getNextSectionName();			
			fieldIndex++;			
		}
	}
}
//--------------------------------------------------------------------------
void ShellDetailsManager::deleteCurrentDirectoryCache()
{
	ZeroMemory (m_currentDirectoryPath, sizeof (m_currentDirectoryPath) * sizeof (TCHAR));	
}
//--------------------------------------------------------------------------
void ShellDetailsManager::deleteCurrentFileNameCache()
{
	ZeroMemory (m_currentPath, sizeof (m_currentPath) * sizeof (TCHAR));
}
//--------------------------------------------------------------------------