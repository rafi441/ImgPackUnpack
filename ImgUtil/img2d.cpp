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

BOOL IMAGE2D::Compress(BYTE* tOriginal, DWORD tOriginalSize, BYTE* tCompress, DWORD &tCompressSize)
{
    uLongf destLen = tCompressSize;

    if (compress(tCompress, &destLen, tOriginal, tOriginalSize) != Z_OK)
    {
        // compression failed
        return FALSE;
    }

    tCompressSize = static_cast<DWORD>(destLen);  // Update with the actual compressed size
    return TRUE;
}

void IMAGE2D::Unpack(std::string tFileName)
{
    printf("%s\n", tFileName.c_str());

    HANDLE hFile = CreateFileA(tFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

    auto size1 = tFileName.find(".\\Original\\");
    if (size1 != std::string::npos)
    {
        tFileName.erase(size1, 11);
    }
    auto size = tFileName.find(".IMG");
    if (size != std::string::npos)
    {
        tFileName.erase(size, 4);
    }
    char tOutputName[1000];

    sprintf(tOutputName, "DDS\\%s.DDS", tFileName.c_str());

    HANDLE hOutputFile = CreateFile(tOutputName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOutputFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        WriteFile(hOutputFile, &uIMG2D->mTexture, uIMG2D->mTextureSize, &bytesWritten, NULL);
        CloseHandle(hOutputFile);
        printf("Texture saved\n");
        sprintf(tOutputName, "DDS\\%s.json", tFileName.c_str());
        WriteCompressData(tOutputName);
    }
    else
    {
        printf("Failed to create the file\n");
    }

    HeapFree(GetProcessHeap(), 0, tOriginal);
    HeapFree(GetProcessHeap(), 0, tCompress);
    CloseHandle(hFile);
}

void IMAGE2D::Repack(std::string tFileName)
{
    json j;
    std::ifstream input(tFileName.c_str());
    if (!input)
    {
        printf("Failed to open the JSON file\n");
        return;
    }
    input >> j;

    auto img = (*uIMG2D);

    img.mTextureInfo.Width = j["mTextureInfo"]["Width"];
    img.mTextureInfo.Height = j["mTextureInfo"]["Height"];
    img.mTextureInfo.Depth = j["mTextureInfo"]["Depth"];
    img.mTextureInfo.MipLevels = j["mTextureInfo"]["MipLevels"];
    img.mTextureInfo.Format = j["mTextureInfo"]["Format"];
    img.mTextureInfo.ResourceType = j["mTextureInfo"]["ResourceType"];
    img.mTextureInfo.ImageFileFormat = j["mTextureInfo"]["ImageFileFormat"];
    img.mLoadFormat = j["mLoadFormat"];
    img.mTextureSize = j["mTextureSize"];
    // Note: mTexture is a IDirect3DTexture9* pointer, so we might need a separate method to handle the loading from file or other data source.
    ParseTexture(tFileName);

    // Modify the filename for the output .IMG file
    auto size1 = tFileName.find(".\\DDS\\");
    if (size1 != std::string::npos)
    {
        tFileName.erase(size1, 6);
    }
    auto size = tFileName.find(".json");
    if (size != std::string::npos)
    {
        tFileName.erase(size, 6);
    }
    tFileName += ".IMG";

    // 2. Compress the data.
    // We'll need a corresponding Compress function to pair with the Decompress function
    // from the Unpack function. For now, I'll assume there's a Compress function
    // with the signature: `BOOL Compress(BYTE* original, DWORD originalSize, BYTE* compressed, DWORD* compressedSize)`

    BYTE* tOriginal = (BYTE*)uIMG2D;  // Assuming uIMG2D points to the full data to be compressed.
    DWORD tOriginalSize = sizeof(IMAGE_FOR_UNCOMPRESS) + img.mTextureSize;  // This assumes the mTextureSize includes everything after the mTexture pointer in the struct.

    DWORD tCompressedSizeEstimate = tOriginalSize * 1.1;  // Assuming compression might not always reduce size. This is just an estimate.
    BYTE* tCompressed = (BYTE*)HeapAlloc(GetProcessHeap(), 0, tCompressedSizeEstimate);
    DWORD tActualCompressedSize;

    if (!Compress(tOriginal, tOriginalSize, tCompressed, tActualCompressedSize))
    {
        printf("Failed to compress data\n");
        HeapFree(GetProcessHeap(), 0, tCompressed);
        return;
    }

    // 3. Write the compressed data to the .IMG file.
    HANDLE hFile = CreateFileA(tFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Failed to create the .IMG file\n");
        HeapFree(GetProcessHeap(), 0, tCompressed);
        return;
    }

    DWORD bytesWritten;
    WriteFile(hFile, &tOriginalSize, sizeof(DWORD), &bytesWritten, NULL);  // Write original size
    WriteFile(hFile, &tActualCompressedSize, sizeof(DWORD), &bytesWritten, NULL);  // Write compressed size
    WriteFile(hFile, tCompressed, tActualCompressedSize, &bytesWritten, NULL);  // Write compressed data

    HeapFree(GetProcessHeap(), 0, tCompressed);
    CloseHandle(hFile);
}

void IMAGE2D::Free()
{
	mCheckValidState = FALSE;

	SAFE_RELEASE(uIMG2D->mTexture);
}

BOOL IMAGE2D::WriteCompressData(std::string tFileName)
{
    json j;
    j["mTextureInfo"]["Width"] = uIMG2D->mTextureInfo.Width;
    j["mTextureInfo"]["Height"] = uIMG2D->mTextureInfo.Height;
    j["mTextureInfo"]["Depth"] = uIMG2D->mTextureInfo.Depth;
    j["mTextureInfo"]["MipLevels"] = uIMG2D->mTextureInfo.MipLevels;
    j["mTextureInfo"]["Format"] = uIMG2D->mTextureInfo.Format; // Note: This might be an enum or a numerical value depending on the API version.
    j["mTextureInfo"]["ResourceType"] = uIMG2D->mTextureInfo.ResourceType; // Similar note applies here.
    j["mTextureInfo"]["ImageFileFormat"] = uIMG2D->mTextureInfo.ImageFileFormat; // And here too.
    j["mLoadFormat"] = uIMG2D->mLoadFormat;
    j["mTextureSize"] = uIMG2D->mTextureSize;
    j["mTexture"] = tFileName.c_str();

    j = j.flatten().unflatten();
    std::ofstream output(tFileName.c_str());
    output << std::setw(4) << j << std::endl;
    printf("Success Write Output\n");
    return TRUE;
}

void IMAGE2D::ReadCompressData(std::string tFileName)
{
    std::string str;
    std::ifstream file;

    try
    {
        file.open(tFileName);
        if (!file.is_open())
            return;
        if (!file.good())
            return;
    }
    catch (std::exception& e)
    {
        throw (e.what());
    }
    std::getline(std::ifstream(tFileName), str, '\0');

    try
    {
        json j = json::parse(str);
        auto img = (*uIMG2D);
        img.mTextureInfo.Width = j["mTextureInfo"]["Width"].get<UINT>();
        img.mTextureInfo.Height = j["mTextureInfo"]["Height"].get<UINT>();
        img.mTextureInfo.Depth = j["mTextureInfo"]["Depth"].get<UINT>();
        img.mTextureInfo.MipLevels = j["mTextureInfo"]["MipLevels"].get<UINT>();
        img.mTextureInfo.Format = j["mTextureInfo"]["Format"].get<D3DFORMAT>();
        img.mTextureInfo.ResourceType = j["mTextureInfo"]["ResourceType"].get<D3DRESOURCETYPE>();
        img.mTextureInfo.ImageFileFormat = j["mTextureInfo"]["ImageFileFormat"].get<D3DXIMAGE_FILEFORMAT>();
        img.mLoadFormat = j["mLoadFormat"].get<D3DFORMAT>();
        img.mTextureSize = j["mTextureSize"].get<int>();
        ParseTexture(tFileName);
    }
    catch (std::exception& e)
    {
        printf("%s\n", e.what());
    }


}

void IMAGE2D::ParseTexture(std::string tFileName)
{
    auto pos = tFileName.find(".json");
    if (pos != std::string::npos)
    {
        tFileName.erase(pos, 6);
    }
    tFileName += ".DDS";
    std::ifstream file(tFileName, std::ios::binary | std::ios::ate);
    if (!file)
    {
        printf("Failed to open the DDS file\n");
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Assuming mTexture is of type BYTE* in this case. If not, adjust accordingly.
    BYTE* buffer = new BYTE[size];

    if (file.read((char*)buffer, size))
    {
        auto a = (*uIMG2D);
        CopyMemory(&a.mTexture, &buffer, size);
        //uIMG2D->mTextureSize = size;
    }
    else
    {
        delete[] buffer;
        printf("Failed to read the DDS file\n");
    }
}