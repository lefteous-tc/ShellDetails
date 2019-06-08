#ifndef ShellDetailsH
#define ShellDetailsH

#include <tchar.h>
#include <windows.h>
#include <strsafe.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <Shlguid.h>

//#include <fstream>
#include <deque>
using namespace std;

#include "contentplug.h"

// typedefs
typedef IShellFolder2	FAR*	LPSHELLFOLDER2;


// structures
struct ColumnInfo
{
	int columnIndex;
	string columnTitle;
	SHCOLUMNID columnID;	
	int columnState;
};

struct ShellFolderDetails
{
	string dir;
	string path;
	//deque <string> details;
	//deque<string> titles;
	bool found;
	int ColumnIndex;	
	//deque<ColumnInfo> columnInfos;
};

//typedef WINSHELLAPI LPITEMIDLIST (WINAPI *pfSHSimpleIDListFromPath)(LPSTR lpszPath);

// Globals
LPSHELLFOLDER2	m_pFolder2;
bool initialized = false;
//ofstream o ("C:\\Log\\sd.log");
deque<ColumnInfo> columnInfos;

// Constants	
const int FIELD_COUNT = 8;

// Enums
enum FieldIndexes {fiArtist, fiSongTitle, fiAlbum, fiYear, fiComment, fiTrack, fiGenre, fiLyrics};

// Arrays
char* FieldNames[FIELD_COUNT] = {"Artist", "Song title", "Album", "Year", "Comment", "Track", "Genre", "Lyrics"};
int FieldTypes[FIELD_COUNT]= {ft_string, ft_string, ft_string, ft_string, ft_string, ft_string, ft_string, ft_string};
int MusicPIDs [8] = {PIDSI_ARTIST, PIDSI_SONGTITLE, PIDSI_ALBUM, PIDSI_YEAR, PIDSI_COMMENT, PIDSI_TRACK, PIDSI_GENRE, PIDSI_LYRICS};

// functions
int getColumns(ShellFolderDetails* pShellFolderDetails, bool b, void* value);	
int getPathColumn (ShellFolderDetails* pShellFolderDetails, void* value);
void getHeader (ShellFolderDetails* pShellFolderDetails);
//void getHeader2 (ShellFolderDetails* pShellFolderDetails);
void simpleInit (ShellFolderDetails* pShellFolderDetails);
LPITEMIDLIST GetNextItemID(LPCITEMIDLIST pidl) ;
void headerInit (ShellFolderDetails* pShellFolderDetails);

/*
The summary information property set is a standard OLE property set that supports the following PIDs.

2 Title VT_LPSTR 
3 Subject VT_LPSTR 
4 Author VT_LPSTR 
5 Keywords VT_LPSTR 
6 Comments VT_LPSTR 
7 Template VT_LPSTR 
8 Last Saved By VT_LPSTR 
9 Revision Number VT_LPSTR 
10 Total Editing Time VT_FILETIME 
11 Last Printed VT_FILETIME 
12 Create Time/Date VT_FILETIME 
13 Last Saved Time/Date VT_FILETIME 
14 Number of Pages VT_I4 
15 Number of Words VT_I4 
16 Number of Characters VT_I4 
17 Thumbnail VT_CF 
18 Name of Creating Application VT_LPSTR 
19 Security VT_I4 


DEFINE_GUID(FMTID_MUSIC, 0x56a3372e, 0xce9c, 0x11d2, 0x9f, 0xe, 0x0, 0x60, 0x97, 0xc6, 0x86, 0xf6);
#define PIDSI_ARTIST    2
#define PIDSI_SONGTITLE 3
#define PIDSI_ALBUM     4
#define PIDSI_YEAR      5
#define PIDSI_COMMENT   6
#define PIDSI_TRACK     7
#define PIDSI_GENRE     11
#define PIDSI_LYRICS    12

//#define FMTID_CadExplorer     {0x17E8AA44, 0xFA9F, 0x4D48, 0xB8, 0x9A, 0x23, 0x39, 0x9A, 0x71, 0xCD, 0xE6}
//DEFINE_GUID(FMTID_CadExplorer, 0x17E8AA44, 0xFA9F, 0x4D48, 0xB8, 0x9A, 0x23, 0x39, 0x9A, 0x71, 0xCD, 0xE6);

*/

#endif