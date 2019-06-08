#ifndef ShellDetailsManagerH
#define ShellDetailsManagerH

#include <tchar.h>
#include <windows.h>

#include <shlobj.h>

#include "ShellColumnInfo.h"
#include "SectionManagerINIFile.h"

#include "Logger.h"

struct USER_DEFINED_NUMBER_FORMAT
{
	double value;
	TCHAR  displayedValue [MAX_PATH];
};

class ShellDetailsManager
{
public:		
	ShellDetailsManager();
	~ShellDetailsManager();

	// Retrieves all required information about the avaialble fields.
	// RETURN: A deque container with all required information about the avaialble fields.
	ShellColumnInfo* getAvailableFields();
	int getAvailableFieldsCount();

	// Sets the directory for extraction is necessary.
	// POST: Directory for extraction is stored in m_currentDirectoryPath.
	void setExtractionDirectory (const TCHAR* path);

	// Sets item ID list for extract operation id necessary.
	// POST: m_pidl is item id list for extraction operation.
	void setExtractionItemIDLIst (const TCHAR* path);

	// Will retrieve get field value for the given index.
	// RETURN: The field type (defined in content
	int extractValue (const int fieldIndex,  void* fieldValue);
	
	// POST: m_pDesktop contains aN IShellFolder pointer to the desktop.
	// POST: 		

	void init();

	void setINIFilePath (TCHAR* iniFilePath);
	TCHAR* getINIFilePath ();

	void deleteCurrentDirectoryCache();
	void deleteCurrentFileNameCache();

private:
	// operations
	int bindCurrentDirectory (TCHAR* currentDirectoryPath);
	int initDirectory (TCHAR* directory);

	int extractStringValue (void* fieldValue, const VARIANT& columnValue, ShellColumnInfo& columnInfo);
	int extractArrayStringValue (void* fieldValue, const VARIANT& columnValue, ShellColumnInfo& columnInfo);
	int extractNumericValue (void* fieldValue, const VARIANT& columnValue, ShellColumnInfo& columnInfo);
	int extractDateValue (void* fieldValue, const VARIANT& pcolumnValue);

	static int bindDirectory (const TCHAR* directoryPath, IShellFolder* pDesktopShellFolder, IShellFolder2* pShellFolder2);

	void getColumnsFromShell ();
	void getColumnsFromINIFile ();

	
	// Retrieve the localized titles.
	// PRE : m_pShellFolder must be != NULL
	// POST: m_columnInfos contains ShellColumnInfo Objects for all fields.

	// members	

	// Points to the directory m_currentPath.
	IShellFolder* m_pCurrentShellFolder;
	IShellFolder* m_pDesktop;

	LPITEMIDLIST m_pidl;
	LPITEMIDLIST m_lastpidl;

	//LPMALLOC m_pMalloc;
	
	// A container which holds informations about available columns.
	ShellColumnInfo* m_pShellColumnInfo;	

	// The current directory.
	TCHAR m_currentDirectoryPath[MAX_PATH];
	TCHAR m_currentPath[MAX_PATH];	
	
	TCHAR m_iniFilePath[MAX_PATH];	

	int m_untitledCount;
	int m_sectionCount;

	Logger m_logger;
};

#endif