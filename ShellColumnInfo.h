#ifndef ShellColumnInfoH
#define ShellColumnInfoH

#include "WinAppHeader.h"

#include <shobjidl.h>

#include "BasicINIFile.h"

/*
#include <string>
using namespace std;
*/

typedef DWORD SHCOLSTATEF;

class ShellColumnInfo
{
public:
	// Constructors
	ShellColumnInfo ();
	ShellColumnInfo (BasicINIFile* iniFilePath);
	ShellColumnInfo (BasicINIFile* iniFilePath, SHCOLUMNID columnID);
	//ShellColumnInfo (char* iniFilePath, SHCOLUMNID columnID);
	
	// Destructor
	~ShellColumnInfo();

	// operations
	void load();
	void save(BOOL asDefault);
	//void getColumnInfoFromSection ();
	int getColumnType() const;

	// set and get operations for members.	
	BOOL getUseDefault();
	void setColumnTitle (TCHAR* columnTitle);
	TCHAR* getColumnTitle();	
	void setColumnState (SHCOLSTATEF columnState);	
	SHCOLSTATEF getColumnState();
	void setColumnID (SHCOLUMNID columnID);
	SHCOLUMNID& getColumnID();
	int getCast();
	TCHAR getCalcOperator();
	int getCalcOperand();
	int getProcessing(); 

	bool existsCurrentColumnID (SHCOLUMNID& columnID);


private:
	
	// members

	// Saved as section name.
	SHCOLUMNID m_columnID;

	// members saved as INI file settings
	BOOL m_useDefault;	
	TCHAR m_columnTitle[MAX_PATH];
	int m_cast;
	TCHAR m_calcOperator;
	int m_calcOperand;
	int m_processing;

	int m_columnState;
	BasicINIFile* m_pIniFile;	
	void filterFieldName(TCHAR* fieldName);

};

#endif