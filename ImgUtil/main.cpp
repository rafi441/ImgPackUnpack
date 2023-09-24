#include "stdafx.h"

int main(int *argc, char *argv[])
{
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind;  // Searching for all .IMG files in the current directory

    IMAGE2D pImg;

    if (strcmp(argv[1], "pack") == 0)
    {
        hFind = FindFirstFileA(".\\DDS\\*.json", &findFileData);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                const std::string file = ".\\DDS\\" + std::string(findFileData.cFileName);
                pImg.Repack(file);
            } while (FindNextFileA(hFind, &findFileData) != 0);

            FindClose(hFind);
        }
    }
    else if (strcmp(argv[1], "unpack") == 0)
    {
        hFind = FindFirstFileA(".\\Original\\*.IMG", &findFileData);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                const std::string file = ".\\Original\\" + std::string(findFileData.cFileName);
                pImg.Unpack(file);
            } 
            while (FindNextFileA(hFind, &findFileData) != 0);

            FindClose(hFind);
        }
    }
    else
    {
        printf("Unknown Command");
    }
    return 0;
}