#include <Windows.h>

class MEMORY_BUFFER
{
public:
	MEMORY_BUFFER() : m_lpBuffer(NULL), m_lpMask(NULL), m_nSize(0) { }

	MEMORY_BUFFER(LPCSTR lpHexStream) : m_lpBuffer(NULL), m_lpMask(NULL), m_nSize(0)
	{
		ReadHexStream(lpHexStream, NULL, NULL, &m_nSize);
		m_lpBuffer = new BYTE [m_nSize];
		m_lpMask = new BYTE[m_nSize];
		ReadHexStream(lpHexStream, m_lpBuffer, m_lpMask, NULL);
	}

	MEMORY_BUFFER(LPBYTE lpBuffer, SIZE_T nSize) : m_lpBuffer(0), m_lpMask(0), m_nSize(nSize)
	{
		m_lpBuffer = new BYTE [m_nSize];
		m_lpMask = new BYTE [m_nSize];
		RtlMoveMemory(m_lpBuffer, lpBuffer, m_nSize);
		RtlFillMemory(m_lpMask, m_nSize, 1);
	}

	~MEMORY_BUFFER()
	{
		if (m_lpBuffer)
			delete[] m_lpBuffer;
		if (m_lpMask)
			delete[] m_lpMask;
	}

	BOOL IsEqual(LPBYTE lpData) const
	{
		for (SIZE_T i = 0; i < m_nSize; i++)
		{
			if (!m_lpMask[i])
				continue;
			if (m_lpBuffer[i] != lpData[i])
				return FALSE;
		}
		return TRUE;
	}

	VOID UpdateData(LPBYTE lpData) const
	{
		for (SIZE_T i = 0; i < m_nSize; i++)
		{
			if (!m_lpMask[i])
				continue;
			lpData[i] = m_lpBuffer[i];
		}
	}

	SIZE_T Size() const
	{
		return m_nSize;
	}

private:
	void ReadHexStream(LPCSTR lpHexStream, LPBYTE lpBuffer, LPBYTE lpMask, SIZE_T * nSize)
	{
		SIZE_T nHexCount = 0;
		SIZE_T nLength = strlen(lpHexStream);

		if (!lpHexStream)
			throw "no input hex stream";

		for (SIZE_T i = 0; i < nLength; i++)
		{
			DWORD firstByte = HexSymbToInt(lpHexStream[i]);
			DWORD secondByte = HexSymbToInt(lpHexStream[i + 1]);

			if (i + 1 < nLength &&
				firstByte >= 0x0 && firstByte <= 0xf &&
				secondByte >= 0x0 && secondByte <= 0xf)
			{
				if (lpBuffer)
					lpBuffer[nHexCount] = firstByte << 4 | secondByte;
				if (lpMask)
					lpMask[nHexCount] = 0x1;
				nHexCount++;
				i++;
			}
			else if (i + 1 < nLength && firstByte == 0x10 && secondByte == 0x10)
			{
				if (lpBuffer)
					lpBuffer[nHexCount] = 0x0;
				if (lpMask)
					lpMask[nHexCount] = 0x0;
				nHexCount++;
				i++;
			}
			else if (firstByte == 0x11)
				continue;
			else
			{
				throw "incorrect input hex stream (example: \"00 ff ab ee\")";
			}
		}

		if (nSize)
		{
			*nSize = nHexCount;
		}
	}

	DWORD HexSymbToInt(char symb)
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

		case '?': // mask
			return 0x10;
		case ' ':
		case '\t':
			return 0x11;
		}
		return -1;
	}

	LPBYTE m_lpBuffer;
	LPBYTE m_lpMask;
	SIZE_T m_nSize;
};