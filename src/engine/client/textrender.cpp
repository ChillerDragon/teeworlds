/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <base/math.h>
#include <engine/textrender.h>

#include "textrender.h"

int CGlyphMap::CAtlas::TrySection(int Index, int Width, int Height)
{
	return 0;
}

void CGlyphMap::CAtlas::Init(int Index, int X, int Y, int Width, int Height)
{
}

int CGlyphMap::AdjustOutlineThicknessToFontSize(int OutlineThickness, int FontSize)
{
	return 0;
}

void CGlyphMap::InitTexture(int Width, int Height)
{
}

int CGlyphMap::FitGlyph(int Width, int Height, ivec2 *pPosition)
{
	return 0;
}

void CGlyphMap::UploadGlyph(int TextureIndex, int PosX, int PosY, int Width, int Height, const unsigned char *pData)
{
}

bool CGlyphMap::SetFaceByName(FT_Face *pFace, const char *pFamilyName)
{
	return false;
}

CGlyphMap::CGlyphMap(IGraphics *pGraphics, FT_Library FtLibrary)
{
}

CGlyphMap::~CGlyphMap()
{
}

int CGlyphMap::GetCharGlyph(int Chr, FT_Face *pFace)
{
	return 0;
}

int CGlyphMap::AddFace(FT_Face Face)
{
	return 0;
}

void CGlyphMap::SetDefaultFaceByName(const char *pFamilyName)
{
}

void CGlyphMap::AddFallbackFaceByName(const char *pFamilyName)
{
}

void CGlyphMap::SetVariantFaceByName(const char *pFamilyName)
{
}

bool CGlyphMap::RenderGlyph(CGlyph *pGlyph, bool Render)
{
	return true;
}



int CGlyphMap::GetFontSizeIndex(int PixelSize) const
{
	return 0;
}

void CGlyphMap::TouchPage(int Index)
{
}

void CGlyphMap::PagesAccessReset()
{
}

void CTextRender::TextRefreshGlyphs(CTextCursor *pCursor)
{
}

int CTextRender::LoadFontCollection(const void *pFilename, const void *pBuf, long FileSize)
{
	return 0;
}

CTextRender::CTextRender()
{
}

void CTextRender::Init()
{
}

void CTextRender::Update()
{
}

void CTextRender::Shutdown()
{
}

void CTextRender::LoadFonts(IStorage *pStorage, IConsole *pConsole)
{
}

void CTextRender::SetFontLanguageVariant(const char *pLanguageFile)
{
}

float CTextRender::TextWidth(float FontSize, const char *pText, int Length)
{
	return 0.0f;
}

void CTextRender::TextDeferred(CTextCursor *pCursor, const char *pText, int Length)
{
}

void CTextRender::TextNewline(CTextCursor *pCursor)
{
}

void CTextRender::TextAdvance(CTextCursor *pCursor, float AdvanceX)
{
}

void CTextRender::DrawText(CTextCursor *pCursor, vec2 Offset, int Texture, bool IsSecondary, float Alpha, int StartGlyph = 0, int NumGlyphs = -1)
{
}

void CTextRender::TextPlain(CTextCursor *pCursor, const char *pText, int Length)
{
}

void CTextRender::TextOutlined(CTextCursor *pCursor, const char *pText, int Length)
{
}

void CTextRender::TextShadowed(CTextCursor *pCursor, const char *pText, int Length, vec2 ShadowOffset)
{
}

void CTextRender::DrawTextPlain(CTextCursor *pCursor, float Alpha, int StartGlyph, int NumGlyphs)
{
}

void CTextRender::DrawTextOutlined(CTextCursor *pCursor, float Alpha, int StartGlyph, int NumGlyphs)
{
}

void CTextRender::DrawTextShadowed(CTextCursor *pCursor, vec2 ShadowOffset, float Alpha, int StartGlyph, int NumGlyphs)
{
}

int CTextRender::CharToGlyph(CTextCursor *pCursor, int NumChars, float *pLineWidth)
{
	return 0;
}

IEngineTextRender *CreateEngineTextRender() { return new CTextRender; }
