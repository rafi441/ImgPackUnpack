#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <d3d9.h>
#include <d3dx9.h>
#include <Windows.h>
#include "zlib/zlib.h"
#include "json.hpp"
using json = nlohmann::json;

#pragma comment(lib, ".//zlib/zdll.lib")

#define SAFE_RELEASE(p) { if(p) { p->Release(); p = NULL; } }
#define HEAP_FREE(p) { if(p){ HeapFree(GetProcessHeap(), 0, p); p = NULL; } }

typedef struct
{
	D3DXIMAGE_INFO mTextureInfo; //offset 0, size = 28
	D3DFORMAT mLoadFormat; //offset 28, size = 4
	int mTextureSize; //offset 32, size = 4
	IDirect3DTexture9* mTexture; //offset 36, size = mTextureSize
} IMAGE_FOR_UNCOMPRESS;

class IMAGE2D
{
public:
	BOOL mCheckValidState;
	IMAGE_FOR_UNCOMPRESS* uIMG2D;
	BOOL Decompress(DWORD tCompressSize, BYTE* tCompress, DWORD tOriginalSize, BYTE* tOriginal);
	BOOL Compress(BYTE* tOriginal, DWORD tOriginalSize, BYTE* tCompress, DWORD &tCompressSize);
	void Unpack(std::string tFileName);
	void Repack(std::string tFileName);
	void Free(void);

	BOOL WriteCompressData(std::string tFileName);
	void ReadCompressData(std::string tFileName);
	void ParseTexture(std::string tFileName);
};
extern IMAGE2D pImg;