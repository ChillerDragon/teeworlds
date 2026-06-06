/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/detect.h>
#include <base/math.h>
#include <base/tl/threading.h>

#include <base/system.h>
#include <engine/external/pnglite/pnglite.h>

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/keys.h>
#include <engine/console.h>

#include <math.h> // cosf, sinf

#include "graphics.h"


#if defined(CONF_PLATFORM_MACOSX)

	class semaphore
	{
		SDL_sem *sem;
	public:
		semaphore() { sem = SDL_CreateSemaphore(0); }
		~semaphore() { SDL_DestroySemaphore(sem); }
		void wait() { SDL_SemWait(sem); }
		void signal() { SDL_SemPost(sem); }
	};
#endif


static CVideoMode g_aFakeModes[] = {
	{320,240,8,8,8}, {400,300,8,8,8}, {640,480,8,8,8},
	{720,400,8,8,8}, {768,576,8,8,8}, {800,600,8,8,8},
	{1024,600,8,8,8}, {1024,768,8,8,8}, {1152,864,8,8,8},
	{1280,768,8,8,8}, {1280,800,8,8,8}, {1280,960,8,8,8},
	{1280,1024,8,8,8}, {1368,768,8,8,8}, {1400,1050,8,8,8},
	{1440,900,8,8,8}, {1440,1050,8,8,8}, {1600,1000,8,8,8},
	{1600,1200,8,8,8}, {1680,1050,8,8,8}, {1792,1344,8,8,8},
	{1800,1440,8,8,8}, {1856,1392,8,8,8}, {1920,1080,8,8,8},
	{1920,1200,8,8,8}, {1920,1440,8,8,8}, {1920,2400,8,8,8},
	{2048,1536,8,8,8},

	{320,240,5,6,5}, {400,300,5,6,5}, {640,480,5,6,5},
	{720,400,5,6,5}, {768,576,5,6,5}, {800,600,5,6,5},
	{1024,600,5,6,5}, {1024,768,5,6,5}, {1152,864,5,6,5},
	{1280,768,5,6,5}, {1280,800,5,6,5}, {1280,960,5,6,5},
	{1280,1024,5,6,5}, {1368,768,5,6,5}, {1400,1050,5,6,5},
	{1440,900,5,6,5}, {1440,1050,5,6,5}, {1600,1000,5,6,5},
	{1600,1200,5,6,5}, {1680,1050,5,6,5}, {1792,1344,5,6,5},
	{1800,1440,5,6,5}, {1856,1392,5,6,5}, {1920,1080,5,6,5},
	{1920,1200,5,6,5}, {1920,1440,5,6,5}, {1920,2400,5,6,5},
	{2048,1536,5,6,5}
};

void CGraphics_OpenGL::Flush()
{
	// Reset pointer
	m_NumVertices = 0;
}

void CGraphics_OpenGL::AddVertices(int Count)
{
	m_NumVertices += Count;
	if((m_NumVertices + Count) >= MAX_VERTICES)
		Flush();
}

void CGraphics_OpenGL::Rotate4(const CPoint &rCenter, CVertex *pPoints)
{
	float c = cosf(m_Rotation);
	float s = sinf(m_Rotation);
	float x, y;
	int i;

	for(i = 0; i < 4; i++)
	{
		x = pPoints[i].m_Pos.x - rCenter.x;
		y = pPoints[i].m_Pos.y - rCenter.y;
		pPoints[i].m_Pos.x = x * c - y * s + rCenter.x;
		pPoints[i].m_Pos.y = x * s + y * c + rCenter.y;
	}
}

unsigned char CGraphics_OpenGL::Sample(int w, int h, const unsigned char *pData, int u, int v, int Offset, int ScaleW, int ScaleH, int Bpp)
{
	int Value = 0;
	return Value/(ScaleW*ScaleH);
}

unsigned char *CGraphics_OpenGL::Rescale(int Width, int Height, int NewWidth, int NewHeight, int Format, const unsigned char *pData)
{
	return nullptr;
}

CGraphics_OpenGL::CGraphics_OpenGL()
{
}

void CGraphics_OpenGL::ClipEnable(int x, int y, int w, int h)
{
}

void CGraphics_OpenGL::ClipDisable()
{
}

void CGraphics_OpenGL::BlendNone()
{
}

void CGraphics_OpenGL::BlendNormal()
{
}

void CGraphics_OpenGL::BlendAdditive()
{
}

void CGraphics_OpenGL::WrapNormal()
{
}

void CGraphics_OpenGL::WrapClamp()
{
}

int CGraphics_OpenGL::MemoryUsage() const
{
	return m_TextureMemoryUsage;
}

void CGraphics_OpenGL::MapScreen(float TopLeftX, float TopLeftY, float BottomRightX, float BottomRightY)
{
}

void CGraphics_OpenGL::GetScreen(float *pTopLeftX, float *pTopLeftY, float *pBottomRightX, float *pBottomRightY)
{
}

void CGraphics_OpenGL::LinesBegin()
{
SetColor(1,1,1,1);
}

void CGraphics_OpenGL::LinesEnd()
{
}

void CGraphics_OpenGL::LinesDraw(const CLineItem *pArray, int Num)
{
}

int CGraphics_OpenGL::UnloadTexture(int Index)
{
	return 0;
}

int CGraphics_OpenGL::LoadTextureRawSub(int TextureID, int x, int y, int Width, int Height, int Format, const void *pData)
{
	return 0;
}

int CGraphics_OpenGL::LoadTextureRaw(int Width, int Height, int Format, const void *pData, int StoreFormat, int Flags)
{
	return 0;
}

// simple uncompressed RGBA loaders
int CGraphics_OpenGL::LoadTexture(const char *pFilename, int StorageType, int StoreFormat, int Flags)
{
	return 0;
}

int CGraphics_OpenGL::LoadPNG(CImageInfo *pImg, const char *pFilename, int StorageType)
{
	return 0;
}

void CGraphics_OpenGL::ScreenshotDirect(const char *pFilename)
{
}

void CGraphics_OpenGL::TextureSet(int TextureID)
{
}

void CGraphics_OpenGL::Clear(float r, float g, float b)
{
}

void CGraphics_OpenGL::QuadsBegin()
{
}

void CGraphics_OpenGL::QuadsEnd()
{
}

void CGraphics_OpenGL::QuadsSetRotation(float Angle)
{
}

void CGraphics_OpenGL::SetColorVertex(const CColorVertex *pArray, int Num)
{
}

void CGraphics_OpenGL::SetColor(float r, float g, float b, float a)
{
}

void CGraphics_OpenGL::QuadsSetSubset(float TlU, float TlV, float BrU, float BrV)
{
}

void CGraphics_OpenGL::QuadsSetSubsetFree(
	float x0, float y0, float x1, float y1,
	float x2, float y2, float x3, float y3)
{
	m_aTexture[0].u = x0; m_aTexture[0].v = y0;
}

void CGraphics_OpenGL::QuadsDraw(CQuadItem *pArray, int Num)
{
}

void CGraphics_OpenGL::QuadsDrawTL(const CQuadItem *pArray, int Num)
{
}

void CGraphics_OpenGL::QuadsDrawFreeform(const CFreeformItem *pArray, int Num)
{
}

void CGraphics_OpenGL::QuadsText(float x, float y, float Size, const char *pText)
{
}

int CGraphics_OpenGL::Init()
{

	return 0;
}

int CGraphics_SDL::TryInit()
{
	return 0;
}


int CGraphics_SDL::InitWindow()
{
	return -1;
}


CGraphics_SDL::CGraphics_SDL()
{
}

int CGraphics_SDL::Init()
{
	return 0;
}

void CGraphics_SDL::Shutdown()
{
}

void CGraphics_SDL::Minimize()
{
}

void CGraphics_SDL::Maximize()
{
}

int CGraphics_SDL::WindowActive()
{
	return 0;
}

int CGraphics_SDL::WindowOpen()
{
	return 0;

}

void CGraphics_SDL::TakeScreenshot(const char *pFilename)
{
}

void CGraphics_SDL::Swap()
{
}


int CGraphics_SDL::GetVideoModes(CVideoMode *pModes, int MaxModes)
{
	return 0;
}

// syncronization
void CGraphics_SDL::InsertSignal(semaphore *pSemaphore)
{
}

bool CGraphics_SDL::IsIdle()
{
	return true;
}

void CGraphics_SDL::WaitForIdle()
{
}

extern IEngineGraphics *CreateEngineGraphics() { return new CGraphics_SDL(); }
