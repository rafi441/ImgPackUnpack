#include <stdio.h>
#include <string>
#include <vector>
#include <cstdint>
#include <d3d9.h>
#include <d3dx9.h>
#include <Windows.h>
#include "zlib/zlib.h"

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
	void Unpack(std::string tFileName);
	void Free(void);
};
extern IMAGE2D pImg;