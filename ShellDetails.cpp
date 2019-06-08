#include <tchar.h>
#include <windows.h>

#include "contentplug.h"
#include "ShellDetailsManager.h"
#include "PluginUtils.h"

ShellDetailsManager* shellDetailsManager = NULL;
HMODULE hinstDLL = NULL;
HANDLE semaphore = NULL;


void contentSetDefaultParams(ContentDefaultParamStruct* dps);
int contentGetSupportedField(int FieldIndex, TCHAR* FieldName, TCHAR* Units, int maxlen);
int contentGetValue(TCHAR* FileName,int FieldIndex,int, void* FieldValue,int,int flags);


BOOL APIENTRY DllMain(HANDLE hDLL, DWORD reason, LPVOID)
{	
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:		
		hinstDLL = (HMODULE)hDLL;
		semaphore = CreateSemaphore (NULL, 1, 1, NULL);		
		shellDetailsManager = new ShellDetailsManager;
		break;
	case DLL_PROCESS_DETACH:
		CloseHandle (semaphore);		
		delete shellDetailsManager;
		shellDetailsManager = NULL;
		break;
	}
    return TRUE;
}

int __stdcall ContentGetSupportedField(int fieldIndex, char* fieldName, char* units, int maxlen)
{
#ifdef UNICODE
	TCHAR fieldNameWide [MAX_PATH] = {0};
	TCHAR unitsWide [MAX_PATH] = {0};
	int result = contentGetSupportedField (fieldIndex, fieldNameWide, unitsWide, maxlen);
	WideCharToMultiByte (CP_ACP, 0, fieldNameWide, -1, fieldName, MAX_PATH, NULL, NULL);	
	WideCharToMultiByte (CP_ACP, 0, unitsWide, -1, units, MAX_PATH, NULL, NULL);
	return result;
#else
	return contentGetSupportedField (fieldIndex, fieldName, units, maxlen);
#endif	
}

int contentGetSupportedField(int FieldIndex, TCHAR* FieldName, TCHAR* Units, int maxlen)
{
	if (FieldIndex < 0 || FieldIndex > shellDetailsManager->getAvailableFieldsCount() -1)
	{		
		return ft_nomorefields;	
	}
	Units[0] = 0;
	ShellColumnInfo* shellColumnInfo = shellDetailsManager->getAvailableFields();
	TCHAR* caption = shellColumnInfo[FieldIndex].getColumnTitle();
	if (!_tcslen(caption))
	{
		return ft_nomorefields;
	}
	StringCchCopy (FieldName, maxlen, caption);

	
	return shellColumnInfo[FieldIndex].getColumnType();	
}

#ifdef UNICODE
int __stdcall ContentGetValueW(TCHAR* FileName,int FieldIndex,int unitIndex, void* FieldValue,int maxlen,int flags)
{	
	return contentGetValue(FileName, FieldIndex, unitIndex, FieldValue, maxlen, flags);
}
#else
int __stdcall ContentGetValue(TCHAR* FileName,int FieldIndex,int unitIndex, void* FieldValue, int maxlen,int flags)
{
	return contentGetValue(FileName, FieldIndex, unitIndex, FieldValue, maxlen, flags);
}
#endif

// Implementation of ContentGetValue.
// PRE:
// POST: FieldValue contains a pointer to ...
int contentGetValue(TCHAR* FileName,int FieldIndex,int, void* FieldValue,int,int flags)
{		
	if (FieldIndex < 0 || FieldIndex > shellDetailsManager->getAvailableFieldsCount() -1)
	{			
		return ft_nosuchfield;	
	}	
	
	if (flags & CONTENT_DELAYIFSLOW)
	{		
		switch (shellDetailsManager->getAvailableFields()[FieldIndex].getProcessing())
		{			
		case 0:			
			// Test if slow bit is set and process and if yes process in background.
			if (shellDetailsManager->getAvailableFields()[FieldIndex].getColumnState() & SHCOLSTATE_SLOW)
			{				
				return ft_delayed;
			}			
		case 1:
			// Foreground
			break;
		case 2: 			
			return ft_delayed;
		case 3:			
			return ft_ondemand;		
		}
	}
	
	int result = 0;
	
	WaitForSingleObject (semaphore, INFINITE);
	shellDetailsManager->setExtractionDirectory (FileName);
	shellDetailsManager->setExtractionItemIDLIst (FileName);
	result = shellDetailsManager->extractValue (FieldIndex, FieldValue);	
	ReleaseSemaphore (semaphore, 1, NULL);		

	return result;
}


void __stdcall ContentSetDefaultParams(ContentDefaultParamStructA* dps)
{	
#ifdef UNICODE
	ContentDefaultParamStructW contentDefaultParamStructWide;
	ZeroMemory (&contentDefaultParamStructWide, sizeof (ContentDefaultParamStructW));
	contentDefaultParamStructWide.size = sizeof (ContentDefaultParamStructW);
	MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, dps->DefaultIniName, -1, contentDefaultParamStructWide.DefaultIniName, MAX_PATH);	
	contentDefaultParamStructWide.PluginInterfaceVersionHi = dps->PluginInterfaceVersionHi;
	contentDefaultParamStructWide.PluginInterfaceVersionLow = dps->PluginInterfaceVersionLow;	
	contentSetDefaultParams(&contentDefaultParamStructWide);
#else
	contentSetDefaultParams(dps);
#endif
}

void contentSetDefaultParams(ContentDefaultParamStruct* dps)
{	
	PluginUtils::getPluginSettingsFilePath (hinstDLL, TEXT("ShellDetails"), dps->DefaultIniName, shellDetailsManager->getINIFilePath(), MAX_PATH);
	shellDetailsManager->init();
}

void __stdcall ContentSendStateInformation(int state, char*)
{
	if (!shellDetailsManager)
	{
		return;
	}
	const int newdir = 1;
	const int refresh = 2;
	if (state & newdir)
	{
		//shellDetailsManager->deleteCurrentDirectoryCache();
		//shellDetailsManager->deleteCurrentFileNameCache();
	}
	if (state & refresh)
	{		
		shellDetailsManager->deleteCurrentDirectoryCache();
		shellDetailsManager->deleteCurrentFileNameCache();
	}
}