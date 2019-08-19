#include "process_patch.h"
#include "memory_buffer.h"

void SpecificHexDump(LPBYTE lpBuffer, SIZE_T nSize, SIZE_T Offset, SIZE_T foundAddress)
{
	SIZE_T LowOffset = 0x0;
	SIZE_T HighOffset = 0x50;

	HexDump(lpBuffer + Offset - LowOffset, HighOffset + LowOffset, foundAddress - LowOffset); 	
	printf("\n");
}

void ToggleThreads(HANDLE hProcess, BOOL State)
{
	typedef LONG (NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
	typedef LONG (NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);

    NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(GetModuleHandle("ntdll"), "NtSuspendProcess");
    NtResumeProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(GetModuleHandle("ntdll"), "NtResumeProcess");

    if (State)
    {
        pfnNtResumeProcess(hProcess);
    } 
	else
    {
        pfnNtSuspendProcess(hProcess);
    }
}

BOOL PatchProcessMemory(HANDLE hProcess, BYTE * bOriginalBuffer, SIZE_T nOrigonalBufferSize, BYTE * bReplaceBuffer, SIZE_T nReplaceBufferSize)
{
	SYSTEM_INFO si = { 0 };
	GetSystemInfo(&si);
	// printf("app address min: %llx\n", si.lpMinimumApplicationAddress);
	// printf("app address max: %llx\n", si.lpMaximumApplicationAddress);

    MEMORY_BASIC_INFORMATION mbi = { 0 };

    ToggleThreads(hProcess, FALSE); // suspend process

	LPCVOID address = si.lpMinimumApplicationAddress;
	while (address < si.lpMaximumApplicationAddress)
	{
		if (!VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)))
		{
			printf("Error: VirtualQueryEx (%d)\n", GetLastError());
			return FALSE;
		}
		// printf("address: %llx size: %d\n", address, mbi.RegionSize);

		if (mbi.State == MEM_COMMIT && ((mbi.Protect & PAGE_GUARD) == 0) && ((mbi.Protect == PAGE_NOACCESS) == 0))
		{
			if ((mbi.Protect & PAGE_READWRITE) != 0 || (mbi.Protect & PAGE_WRITECOPY) != 0 || (mbi.Protect & PAGE_EXECUTE_READWRITE) != 0 || (mbi.Protect & PAGE_EXECUTE_WRITECOPY) != 0)
			{
				BYTE * bRegionMemoryBuffer = new BYTE [mbi.RegionSize];
				RtlFillMemory(bRegionMemoryBuffer, mbi.RegionSize, 0);
				ReadProcessMemory(hProcess, mbi.BaseAddress, bRegionMemoryBuffer, mbi.RegionSize, NULL);
				for (SIZE_T nRegionOffset = 0; nRegionOffset < mbi.RegionSize - nOrigonalBufferSize; nRegionOffset++)
				{
					if (memcmp(bRegionMemoryBuffer + nRegionOffset, bOriginalBuffer, nOrigonalBufferSize) == 0)
					{
						SIZE_T foundAddress = (SIZE_T)address + nRegionOffset;
						printf("Found at address: %08x\n", foundAddress);

						SpecificHexDump(bRegionMemoryBuffer, mbi.RegionSize, nRegionOffset, foundAddress);
						//HexDump(bRegionMemoryBuffer + nRegionOffset , 0x50, (SIZE_T)address + nRegionOffset);
						//HexDump(bOriginalBuffer, nOrigonalBufferSize);
						//HexDump(bReplaceBuffer, nReplaceBufferSize);
						if (bReplaceBuffer && !WriteProcessMemory(hProcess, (LPVOID)foundAddress, bReplaceBuffer, nReplaceBufferSize, NULL))
						{
							printf("Error: WriteProcessMemory (%d)\n", GetLastError());
							//return FALSE;
						}
					}
				}
				delete []bRegionMemoryBuffer;
			}
		}
		address = (LPCVOID)((SIZE_T)address + mbi.RegionSize);
	}

    ToggleThreads(hProcess, TRUE); // resume process
	return TRUE;
}

int main(int argc, char * argv[])
{
    BYTE *bOriginalBuffer = NULL, *bReplaceBuffer = NULL;
    SIZE_T nOriginalBufferSize = 0, nReplaceBufferSize = 0;
    DWORD dwPid = 0;

	if (!ParseArgs(argc, argv, dwPid, bOriginalBuffer, nOriginalBufferSize, bReplaceBuffer, nReplaceBufferSize))
    {
        printf("Error: parse input args\n");
		return -1;
    }
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	if (hProcess == 0)
    {
        printf("Error: OpenProcess with pid %d (%d)", dwPid, GetLastError());
		return -1;
    }

	PatchProcessMemory(hProcess, bOriginalBuffer, sizeof(bOriginalBuffer), bReplaceBuffer, sizeof(bReplaceBuffer));

	CloseHandle(hProcess);
	return 0;
}