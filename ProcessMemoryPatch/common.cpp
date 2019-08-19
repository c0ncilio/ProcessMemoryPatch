#include "process_patch.h"

DWORD GetProcessIdByName(LPCSTR lpProcessName)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (stricmp(entry.szExeFile, lpProcessName) == 0)
            {  
               return entry.th32ProcessID;
            }
        }
    }
    CloseHandle(snapshot);
    return 0;
}


BOOL SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
    ) 
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if ( !LookupPrivilegeValue( 
            NULL,            // lookup privilege on local system
            lpszPrivilege,   // privilege to lookup 
            &luid ) )        // receives LUID of privilege
    {
        printf("LookupPrivilegeValue error: %u\n", GetLastError() ); 
        return FALSE; 
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.

    if ( !AdjustTokenPrivileges(
           hToken, 
           FALSE, 
           &tp, 
           sizeof(TOKEN_PRIVILEGES), 
           (PTOKEN_PRIVILEGES) NULL, 
           (PDWORD) NULL) )
    { 
          printf("AdjustTokenPrivileges error: %u\n", GetLastError() ); 
          return FALSE; 
    } 

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

    {
          printf("The token does not have the specified privilege. \n");
          return FALSE;
    } 

    return TRUE;
}

BOOL FindModuleInProcess(DWORD dwPid, LPCSTR lpModuleName)
{
	DWORD cbNeeded = 0;
	HMODULE hModules[0x1000];
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, dwPid);
	if (!hProcess)
		return FALSE;
	if (!EnumProcessModulesEx(hProcess, hModules, sizeof(hModules), &cbNeeded, LIST_MODULES_ALL))
		return FALSE;
	for (int i = 0; i < cbNeeded / sizeof(HMODULE); i++)
	{
		char ModuleName[0x1000];
		GetModuleFileNameExA(hProcess, hModules[i], ModuleName, sizeof(ModuleName));
		if (stricmp(ModuleName, lpModuleName) == 0)
			return TRUE;
		//printf("%s\n", ModuleName);
	}
	CloseHandle(hProcess);
	return FALSE;
}

DWORD GetProcessIdByModuleName(LPCSTR lpProcessName, LPCSTR lpModuleName)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (stricmp(entry.szExeFile, lpProcessName) == 0)
            {
                if (FindModuleInProcess(entry.th32ProcessID, lpModuleName))
                    return entry.th32ProcessID;
            }
        }
    }
    CloseHandle(snapshot);
    return 0;
}