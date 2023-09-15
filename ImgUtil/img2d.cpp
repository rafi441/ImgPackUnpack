#include "stdafx.h"

IMAGE2D pImg;

BOOL IMAGE2D::Decompress(DWORD tCompressSize, BYTE* tCompress, DWORD tOriginalSize, BYTE* tOriginal)
{
	uLongf to = tOriginalSize;
	if ((uncompress(tOriginal, (uLongf*)&to, tCompress, tCompressSize)) != 0)
	{
		return FALSE;
	}

	return TRUE;
}

void IMAGE2D::Unpack(char* tFileName)
{
    HANDLE hFile = CreateFileA(tFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        Free();
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE)
    {
        CloseHandle(hFile);
        Free();
        return;
    }

    BYTE* tCompress = (BYTE*)HeapAlloc(GetProcessHeap(), 0, fileSize);
    if (!tCompress)
    {
        CloseHandle(hFile);
        Free();
        return;
    }

    DWORD bytesRead;
    if (!ReadFile(hFile, tCompress, fileSize, &bytesRead, NULL) || bytesRead != fileSize)
    {
        HeapFree(GetProcessHeap(), 0, tCompress);
        CloseHandle(hFile);
        Free();
        return;
    }

    DWORD tOriginalSize = *(DWORD*)&tCompress[0];
    DWORD tCompressSize = *(DWORD*)&tCompress[4];

    BYTE* tOriginal = (BYTE*)HeapAlloc(GetProcessHeap(), 0, tOriginalSize);
    if (!tOriginal)
    {
        HeapFree(GetProcessHeap(), 0, tCompress);
        CloseHandle(hFile);
        Free();
        return;
    }

    if (!Decompress(tCompressSize, &tCompress[8], tOriginalSize, tOriginal))
    {
        HeapFree(GetProcessHeap(), 0, tOriginal);
        HeapFree(GetProcessHeap(), 0, tCompress);
        CloseHandle(hFile);
        Free();
        return;
    }

    uIMG2D = (IMAGE_FOR_UNCOMPRESS*)tOriginal;

    HANDLE hOutputFile = CreateFile("blabla.DDS", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOutputFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        WriteFile(hOutputFile, &uIMG2D->mTexture, uIMG2D->mTextureSize, &bytesWritten, NULL);
        CloseHandle(hOutputFile);
        printf("Texture saved to blabla.DDS\n");
    }
    else
    {
        printf("Failed to create the file\n");
    }

    HeapFree(GetProcessHeap(), 0, tOriginal);
    HeapFree(GetProcessHeap(), 0, tCompress);
    CloseHandle(hFile);
}

void IMAGE2D::Free()
{
	mCheckValidState = FALSE;

	SAFE_RELEASE(uIMG2D->mTexture);
}