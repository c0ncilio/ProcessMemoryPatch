#include <Windows.h>
#include <WinDNS.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <stdio.h>
#include <string.h>

void Help();
int HexSymbToInt(char symb);
void ConvertToBinaryBuffer(const char * Data, BYTE*& Buffer, SIZE_T& Size);
void HexDump(BYTE * bBuffer, SIZE_T nBufferSize, DWORD dwAddress);
BOOL ParseArgs(int argc, char * argv[], DWORD& dwPid, BYTE*& bOriginalBuffer, SIZE_T& nOriginalBufferSize, BYTE*& bReplaceBuffer, SIZE_T& nReplaceBufferSize);

DWORD GetProcessIdByName(LPCSTR lpProcessName);
BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
BOOL FindModuleInProcess(DWORD dwPid, LPCSTR lpModuleName);
DWORD GetProcessIdByModuleName(LPCSTR lpProcessName, LPCSTR lpModuleName);

void DNSQuery(const wchar_t * hostname, unsigned char * ip);