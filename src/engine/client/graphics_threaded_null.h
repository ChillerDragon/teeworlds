/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_GRAPHICS_THREADED_NULL_H
#define ENGINE_CLIENT_GRAPHICS_THREADED_NULL_H

#include <engine/graphics.h>

class CGraphics_ThreadedNull : public IEngineGraphics
{
	enum
	{
		NUM_CMDBUFFERS = 2,

		MAX_VERTICES = 32*1024,
		MAX_TEXTURES = 1024*4,
		
		DRAWING_QUADS=1,
		DRAWING_LINES=2
	};

	unsigned m_CurrentCommandBuffer;

	//
	class IStorage *m_pStorage;
	class IConsole *m_pConsole;

	int m_NumVertices;

	bool m_RenderEnable;

	float m_Rotation;
	int m_Drawing;
	bool m_DoScreenshot;
	char m_aScreenshotName[128];

	int m_InvalidTexture;

	int m_aTextureIndices[MAX_TEXTURES];
	int m_FirstFreeTexture;
	int m_TextureMemoryUsage;

	void FlushVertices() {};
	void AddVertices(int Count) {};

	void KickCommandBuffer() {};

	int IssueInit();
	int InitWindow();
public:
	CGraphics_ThreadedNull() {};

	virtual void ClipEnable(int x, int y, int w, int h) {};
	virtual void ClipDisable() {};

	virtual void BlendNone() {};
	virtual void BlendNormal() {};
	virtual void BlendAdditive() {};

	virtual void WrapNormal() {};
	virtual void WrapClamp() {};

	virtual int MemoryUsage() const { return 0; };

	virtual void MapScreen(float TopLeftX, float TopLeftY, float BottomRightX, float BottomRightY) {};
	virtual void GetScreen(float *pTopLeftX, float *pTopLeftY, float *pBottomRightX, float *pBottomRightY) {};

	virtual void LinesBegin() {};
	virtual void LinesEnd() {};
	virtual void LinesDraw(const CLineItem *pArray, int Num) {};

	virtual int UnloadTexture(int Index) { return 0; }
	virtual int LoadTextureRaw(int Width, int Height, int Format, const void *pData, int StoreFormat, int Flags) { return 0; }
	virtual int LoadTextureRawSub(int TextureID, int x, int y, int Width, int Height, int Format, const void *pData) { return 0; }

	// simple uncompressed RGBA loaders
	virtual int LoadTexture(const char *pFilename, int StorageType, int StoreFormat, int Flags) { return 0; }
	virtual int LoadPNG(CImageInfo *pImg, const char *pFilename, int StorageType) { return 0; }

	void ScreenshotDirect(const char *pFilename) {};

	virtual void TextureSet(int TextureID) {};

	virtual void Clear(float r, float g, float b) {};

	virtual void QuadsBegin() {};
	virtual void QuadsEnd() {};
	virtual void QuadsSetRotation(float Angle) {};

	virtual void SetColorVertex(const CColorVertex *pArray, int Num) {};
	virtual void SetColor(float r, float g, float b, float a) {};

	virtual void QuadsSetSubset(float TlU, float TlV, float BrU, float BrV) {};
	virtual void QuadsSetSubsetFree(
		float x0, float y0, float x1, float y1,
		float x2, float y2, float x3, float y3) {};

	virtual void QuadsDraw(CQuadItem *pArray, int Num) {}
	virtual void QuadsDrawTL(const CQuadItem *pArray, int Num) {}
	virtual void QuadsDrawFreeform(const CFreeformItem *pArray, int Num) {}
	virtual void QuadsText(float x, float y, float Size, const char *pText) {}

	virtual void Minimize() {}
	virtual void Maximize() {}

	virtual int WindowActive() { return 0; }
	virtual int WindowOpen() { return 0; }

	virtual int Init() { return 0; }
	virtual void Shutdown() {}

	virtual void TakeScreenshot(const char *pFilename) {}
	virtual void Swap() {}

	virtual int GetVideoModes(CVideoMode *pModes, int MaxModes) { return 0; }

	// syncronization
	virtual void InsertSignal(semaphore *pSemaphore) {};
	virtual bool IsIdle() { return false; };
	virtual void WaitForIdle() {}
};

#endif // ENGINE_CLIENT_GRAPHICS_THREADED_NULL_H
