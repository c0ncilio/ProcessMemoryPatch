#include "process_patch.h"

void Help()
{
	printf("Usage:\n");
	printf("proc_scan.exe -pid 1234 -original \"ff ff ff ff\" -replace \"00 00 00 00\"\n");
	printf("proc_scan.exe -proc process.exe -original \"ff ff ff ff\" -replace \"00 00 00 00\"\n");
}

int HexSymbToInt(char symb)
{
	switch (symb)
	{
	case '0':
		return 0x0;
	case '1':
		return 0x1;
	case '2':
		return 0x2;
	case '3':
		return 0x3;
	case '4':
		return 0x4;
	case '5':
		return 0x5;
	case '6':
		return 0x6;
	case '7':
		return 0x7;
	case '8':
		return 0x8;
	case '9':
		return 0x9;
	case 'a':
	case 'A':
		return 0xa;
	case 'b':
	case 'B':
		return 0xb;
	case 'c':
	case 'C':
		return 0xc;
	case 'd':
	case 'D':
		return 0xd;
	case 'e':
	case 'E':
		return 0xe;
	case 'f':
	case 'F':
		return 0xf;
	}
	return -1;
}

void ConvertToBinaryBuffer(const char * Data, BYTE*& Buffer, SIZE_T& Size)
{
	const char HexSymb[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F' };
	unsigned int Length = strlen(Data);
	int i = 0;
	Size = 0;
	Buffer = new BYTE[Length/2];
	while (i < Length - 1)
	{
		if (Data[i] == ' ')
		{
			i++;
			continue;
		}
		int high = HexSymbToInt(Data[i]);
		int low = HexSymbToInt(Data[i + 1]);
		if (high == -1 || low == -1)
		{
			delete[] Buffer;
			Buffer = NULL;
			Size = 0;
			return;
		}
		unsigned char byte = high << 4 | low;
		Buffer[Size++] = byte;
		i += 2;
	}
}

void HexDump(BYTE * bBuffer, SIZE_T nBufferSize, DWORD dwAddress = 0)
{
	const SIZE_T nColumnInRow = 0x10;
	for (SIZE_T i = 0; i < nBufferSize; i+= nColumnInRow)
	{
		printf("%08x: ", dwAddress + i);
		for (SIZE_T j = 0; j < nColumnInRow; j++)
		{
			if (i + j < nBufferSize)
				printf("%02x ", bBuffer[i+j]);
			else
				printf("   ");
		}
		printf("  ");
		for (SIZE_T j = 0; j < nColumnInRow && i+j  < nBufferSize; j++)
		{
			printf("%c", (bBuffer[i+j] >= 0x20 && bBuffer[i+j] < 0x7f) ? bBuffer[i+j] : '.');
		}
		printf("\n");
	}
}

BOOL ParseArgs(int argc, char * argv[], DWORD& dwPid, BYTE*& bOriginalBuffer, SIZE_T& nOriginalBufferSize, BYTE*& bReplaceBuffer, SIZE_T& nReplaceBufferSize)
{
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-proc") == 0 && i + 1 < argc) // process name
		{
			dwPid = GetProcessIdByName(argv[++i]);
		}
		else if (strcmp(argv[i], "-pid") == 0 && i + 1 < argc) // process id
		{
			dwPid = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-original") == 0 && i + 1 < argc) // original bytes
		{
			ConvertToBinaryBuffer(argv[++i], bOriginalBuffer, nOriginalBufferSize);
		}
		else if (strcmp(argv[i], "-replace") == 0 && i + 1 < argc) // replace bytes
		{
			ConvertToBinaryBuffer(argv[++i], bReplaceBuffer, nReplaceBufferSize);
		}
		else
		{
			Help();
			return FALSE;
		}
	}

	printf("Pid: %d\n", dwPid);
	if (dwPid == 0)
	{
		printf("error: incorrect pid or process name\n");
		return FALSE;
	}
	if (!bOriginalBuffer)
	{
		printf("error: incorrect origin or replace buffer\n");
		return FALSE;
	}

//	BYTE bOriginalBuffer[] = { 0x43, 0x00, 0x61, 0x00, 0x6c, 0x00, 0x63, 0x00, 0x75, 0x00, 0x6c, 0x00, 0x61, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00 };
//	BYTE bReplaceBuffer[]  = { 0x41, 0x00, 0x61, 0x00, 0x6c, 0x00, 0x63, 0x00, 0x75, 0x00, 0x6c, 0x00, 0x61, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00 };
	printf("Original Buffer:\n");
	HexDump(bOriginalBuffer, nOriginalBufferSize);
	printf("Replace Buffer:\n");
	HexDump(bReplaceBuffer, nReplaceBufferSize);

	return TRUE;
}