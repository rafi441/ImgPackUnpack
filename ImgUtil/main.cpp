#include "stdafx.h"

int main(int *argc, char *argv[])
{
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA(".\\Original\\*.IMG", &findFileData);  // Searching for all .IMG files in the current directory

    IMAGE2D pImg;

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            const std::string file = ".\\Original\\" + std::string(findFileData.cFileName);
            pImg.Unpack(file);
        } while (FindNextFileA(hFind, &findFileData) != 0);

        FindClose(hFind);
    }

    return 0;
}