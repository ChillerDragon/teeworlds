/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/color.h>
#include <base/math.h>

#include <engine/client.h>
#include <engine/console.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/storage.h>

#include <generated/client_data.h>
#include <game/client/localization.h>
#include <game/client/render.h>
#include "editor.h"

const char *pDefaultLayerName = "Tiles";

CLayerTiles::CLayerTiles(int w, int h)
{
	m_Type = LAYERTYPE_TILES;
	str_copy(m_aName, pDefaultLayerName, sizeof(m_aName));
	m_Width = w;
	m_Height = h;
	m_Image = -1;
	m_Game = 0;
	m_Color.r = 255;
	m_Color.g = 255;
	m_Color.b = 255;
	m_Color.a = 255;
	m_ColorEnv = -1;
	m_ColorEnvOffset = 0;

	m_pTiles = new CTile[m_Width*m_Height];
	mem_zero(m_pTiles, m_Width*m_Height*sizeof(CTile));

	m_pSaveTiles = 0;
	m_SaveTilesSize = 0;

	m_SelectedRuleSet = 0;
	m_LiveAutoMap = false;
	m_SelectedAmount = 50;
}

CLayerTiles::~CLayerTiles()
{
	delete [] m_pTiles;
	m_pTiles = 0;
	delete [] m_pSaveTiles;
	m_pSaveTiles = 0;
	m_SaveTilesSize = 0;
}

void CLayerTiles::PrepareForSave()
{
	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width; x++)
		{
			m_pTiles[y*m_Width+x].m_Flags &= TILEFLAG_VFLIP|TILEFLAG_HFLIP|TILEFLAG_ROTATE;
			if(m_pTiles[y*m_Width+x].m_Index == 0)
				m_pTiles[y*m_Width+x].m_Flags = 0;
		}

	if(m_Image != -1 && m_Color.a == 255)
	{
		for(int y = 0; y < m_Height; y++)
			for(int x = 0; x < m_Width; x++)
				m_pTiles[y*m_Width+x].m_Flags |= m_pEditor->m_Map.m_lImages[m_Image]->m_aTileFlags[m_pTiles[y*m_Width+x].m_Index];
	}

	int NumSaveTiles = 0; // number of unique tiles that we have to save
	CTile Tile; // current tile to be duplicated
	Tile.m_Skip = MAX_SKIP; // tell the code that we can't skip the first tile

	int NumHitMaxSkip = -1;

	for(int i = 0; i < m_Width * m_Height; i++)
	{
		// we can only store MAX_SKIP empty tiles in one tile
		if(Tile.m_Skip == MAX_SKIP)
		{
			Tile = m_pTiles[i];
			Tile.m_Skip = 0;
			NumSaveTiles++;
			NumHitMaxSkip++;
		}
		// tile is different from last one? - can't skip it
		else if(m_pTiles[i].m_Index != Tile.m_Index || m_pTiles[i].m_Flags != Tile.m_Flags)
		{
			Tile = m_pTiles[i];
			Tile.m_Skip = 0;
			NumSaveTiles++;
		}
		// if the tile is the same as the previous one - no need to
		// save it separately
		else
			Tile.m_Skip++;
	}

	if(m_pSaveTiles)
		delete [] m_pSaveTiles;

	m_pSaveTiles = new CTile[NumSaveTiles];
	m_SaveTilesSize = sizeof(CTile) * NumSaveTiles;

	int NumWrittenSaveTiles = 0;
	Tile.m_Skip = MAX_SKIP;
	for(int i = 0; i < m_Width * m_Height + 1; i++)
	{
		// again, if an tile is the same as the previous one
		// and we have place to store it, skip it!
		// if we are at the end of the layer, write one more tile
		if(i != m_Width * m_Height && Tile.m_Skip != MAX_SKIP && m_pTiles[i].m_Index == Tile.m_Index && m_pTiles[i].m_Flags == Tile.m_Flags)
		{
			Tile.m_Skip++;
		}
		// tile is not skippable
		else
		{
			// if this is not the first tile, we have to save the previous
			// tile beforehand
			if(i != 0)
				m_pSaveTiles[NumWrittenSaveTiles++] = Tile;

			// if this isn't the last tile, store it so we can check how
			// many tiles to skip
			if(i != m_Width * m_Height)
			{
				Tile = m_pTiles[i];
				Tile.m_Skip = 0;
			}
		}
	}
}

void CLayerTiles::ExtractTiles(CTile *pSavedTiles)
{
	int i = 0;
	while(i < m_Width * m_Height)
	{
		for(unsigned Counter = 0; Counter <= pSavedTiles->m_Skip && i < m_Width * m_Height; Counter++)
		{
			m_pTiles[i] = *pSavedTiles;
			m_pTiles[i++].m_Skip = 0;
		}

		pSavedTiles++;
	}
}

void CLayerTiles::MakePalette()
{
	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width; x++)
			m_pTiles[y*m_Width+x].m_Index = y*16+x;
}

void CLayerTiles::Render()
{
	if(m_Image >= 0 && m_Image < m_pEditor->m_Map.m_lImages.size())
		m_Texture = m_pEditor->m_Map.m_lImages[m_Image]->m_Texture;
	Graphics()->TextureSet(m_Texture);
	vec4 Color = vec4(m_Color.r/255.0f, m_Color.g/255.0f, m_Color.b/255.0f, m_Color.a/255.0f);
	Graphics()->BlendNone();
	m_pEditor->RenderTools()->RenderTilemap(m_pTiles, m_Width, m_Height, 32.0f, Color, LAYERRENDERFLAG_OPAQUE,
												m_pEditor->EnvelopeEval, m_pEditor, m_ColorEnv, m_ColorEnvOffset);
	Graphics()->BlendNormal();
	m_pEditor->RenderTools()->RenderTilemap(m_pTiles, m_Width, m_Height, 32.0f, Color, LAYERRENDERFLAG_TRANSPARENT,
												m_pEditor->EnvelopeEval, m_pEditor, m_ColorEnv, m_ColorEnvOffset);
}

int CLayerTiles::ConvertX(float x) const { return (int)(x/32.0f); }
int CLayerTiles::ConvertY(float y) const { return (int)(y/32.0f); }

void CLayerTiles::Convert(CUIRect Rect, RECTi *pOut)
{
	pOut->x = ConvertX(Rect.x);
	pOut->y = ConvertY(Rect.y);
	pOut->w = ConvertX(Rect.x+Rect.w+31) - pOut->x;
	pOut->h = ConvertY(Rect.y+Rect.h+31) - pOut->y;
}

void CLayerTiles::Snap(CUIRect *pRect)
{
	RECTi Out;
	Convert(*pRect, &Out);
	pRect->x = Out.x*32.0f;
	pRect->y = Out.y*32.0f;
	pRect->w = Out.w*32.0f;
	pRect->h = Out.h*32.0f;
}

void CLayerTiles::Clamp(RECTi *pRect)
{
	if(pRect->x < 0)
	{
		pRect->w += pRect->x;
		pRect->x = 0;
	}

	if(pRect->y < 0)
	{
		pRect->h += pRect->y;
		pRect->y = 0;
	}

	if(pRect->x+pRect->w > m_Width)
		pRect->w = m_Width - pRect->x;

	if(pRect->y+pRect->h > m_Height)
		pRect->h = m_Height - pRect->y;

	if(pRect->h < 0)
		pRect->h = 0;
	if(pRect->w < 0)
		pRect->w = 0;
}

void CLayerTiles::BrushSelecting(CUIRect Rect)
{
	vec4 FillColor = HexToRgba(m_pEditor->Config()->m_EdColorSelectionTile);

	Graphics()->TextureClear();
	m_pEditor->Graphics()->QuadsBegin();
	m_pEditor->Graphics()->SetColor(FillColor.r*FillColor.a, FillColor.g*FillColor.a, FillColor.b*FillColor.a, FillColor.a);
	Snap(&Rect);
	IGraphics::CQuadItem QuadItem(Rect.x, Rect.y, Rect.w, Rect.h);
	m_pEditor->Graphics()->QuadsDrawTL(&QuadItem, 1);
	m_pEditor->Graphics()->QuadsEnd();
	char aBuf[16];
	str_format(aBuf, sizeof(aBuf), "%d,%d", ConvertX(Rect.w), ConvertY(Rect.h));
	static CTextCursor s_Cursor;
	s_Cursor.m_FontSize = m_pEditor->m_ShowTilePicker?15.0f:15.0f*m_pEditor->m_WorldZoom;
	s_Cursor.MoveTo(Rect.x+3.0f, Rect.y+3.0f);
	s_Cursor.Reset();
	TextRender()->TextOutlined(&s_Cursor, aBuf, -1);
}

static int s_lastBrushX = -1, s_lastBrushY = -1;
int CLayerTiles::BrushGrab(CLayerGroup *pBrush, CUIRect Rect)
{
	RECTi r;
	Convert(Rect, &r);
	Clamp(&r);

	if(!r.w || !r.h)
		return 0;

	// create new layers
	CLayerTiles *pGrabbed = new CLayerTiles(r.w, r.h);
	pGrabbed->m_pEditor = m_pEditor;
	pGrabbed->m_Texture = m_Texture;
	pGrabbed->m_Image = m_Image;
	pGrabbed->m_Game = m_Game;
	pBrush->AddLayer(pGrabbed);

	// copy the tiles
	for(int y = 0; y < r.h; y++)
		for(int x = 0; x < r.w; x++)
			pGrabbed->m_pTiles[y*pGrabbed->m_Width+x] = m_pTiles[(r.y+y)*m_Width+(r.x+x)];

	s_lastBrushX = -1;
	s_lastBrushY = -1;
	return 1;
}

void CLayerTiles::FillSelection(bool Empty, CLayer *pBrush, CUIRect Rect)
{
	if(m_Readonly)
		return;

	Snap(&Rect);

	int sx = ConvertX(Rect.x);
	int sy = ConvertY(Rect.y);
	int w = ConvertX(Rect.w);
	int h = ConvertY(Rect.h);

	CLayerTiles *pLt = static_cast<CLayerTiles*>(pBrush);

	for(int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			int fx = x+sx;
			int fy = y+sy;

			if(fx < 0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			if(Empty)
				m_pTiles[fy*m_Width+fx].m_Index = 1;
			else
				m_pTiles[fy*m_Width+fx] = pLt->m_pTiles[(y*pLt->m_Width + x%pLt->m_Width) % (pLt->m_Width*pLt->m_Height)];
		}
	}

	if(m_LiveAutoMap)
	{
		RECTi r = {sx - 1, sy - 1, w + 2, h + 2};
		m_pEditor->m_Map.m_lImages[m_Image]->m_pAutoMapper->Proceed(this, m_SelectedRuleSet, r);
	}

	m_pEditor->m_Map.m_Modified = true;
}

void CLayerTiles::BrushDraw(CLayer *pBrush, float wx, float wy)
{
	if(m_Readonly)
		return;

	CLayerTiles *l = (CLayerTiles *)pBrush;
	int sx = ConvertX(wx);
	int sy = ConvertY(wy);

	//dont draw if the mouse is held without moving
	if(sx == s_lastBrushX && sy == s_lastBrushY)
		return;

	for(int y = 0; y < l->m_Height; y++)
		for(int x = 0; x < l->m_Width; x++)
		{
			int fx = x+sx;
			int fy = y+sy;
			if(fx<0 || fx >= m_Width || fy < 0 || fy >= m_Height)
				continue;

			m_pTiles[fy*m_Width+fx] = l->m_pTiles[y*l->m_Width+x];
		}

	if(m_LiveAutoMap)
	{
		RECTi r = {sx - 1, sy - 1, l->m_Width + 2, l->m_Height + 2};
		m_pEditor->m_Map.m_lImages[m_Image]->m_pAutoMapper->Proceed(this, m_SelectedRuleSet, r);
	}

	m_pEditor->m_Map.m_Modified = true;

	s_lastBrushX = sx;
	s_lastBrushY = sy;
}

void CLayerTiles::BrushFlipX()
{
	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width/2; x++)
		{
			CTile Tmp = m_pTiles[y*m_Width+x];
			m_pTiles[y*m_Width+x] = m_pTiles[y*m_Width+m_Width-1-x];
			m_pTiles[y*m_Width+m_Width-1-x] = Tmp;
		}

	if(!m_Game)
		for(int y = 0; y < m_Height; y++)
			for(int x = 0; x < m_Width; x++)
				m_pTiles[y*m_Width+x].m_Flags ^= m_pTiles[y*m_Width+x].m_Flags&TILEFLAG_ROTATE ? TILEFLAG_HFLIP : TILEFLAG_VFLIP;

	s_lastBrushX = -1;
	s_lastBrushY = -1;
}

void CLayerTiles::BrushFlipY()
{
	for(int y = 0; y < m_Height/2; y++)
		for(int x = 0; x < m_Width; x++)
		{
			CTile Tmp = m_pTiles[y*m_Width+x];
			m_pTiles[y*m_Width+x] = m_pTiles[(m_Height-1-y)*m_Width+x];
			m_pTiles[(m_Height-1-y)*m_Width+x] = Tmp;
		}

	if(!m_Game)
		for(int y = 0; y < m_Height; y++)
			for(int x = 0; x < m_Width; x++)
				m_pTiles[y*m_Width+x].m_Flags ^= m_pTiles[y*m_Width+x].m_Flags&TILEFLAG_ROTATE ? TILEFLAG_VFLIP : TILEFLAG_HFLIP;

	s_lastBrushX = -1;
	s_lastBrushY = -1;
}

void CLayerTiles::BrushRotate(float Amount)
{
	int Rotation = (round_to_int(360.0f*Amount/(pi*2))/90)%4;	// 0=0°, 1=90°, 2=180°, 3=270°
	if(Rotation < 0)
		Rotation +=4;

	if(Rotation == 1 || Rotation == 3)
	{
		// 90° rotation
		CTile *pTempData = new CTile[m_Width*m_Height];
		mem_copy(pTempData, m_pTiles, m_Width*m_Height*sizeof(CTile));
		CTile *pDst = m_pTiles;
		for(int x = 0; x < m_Width; ++x)
			for(int y = m_Height-1; y >= 0; --y, ++pDst)
			{
				*pDst = pTempData[y*m_Width+x];
				if(!m_Game)
				{
					if(pDst->m_Flags&TILEFLAG_ROTATE)
						pDst->m_Flags ^= (TILEFLAG_HFLIP|TILEFLAG_VFLIP);
					pDst->m_Flags ^= TILEFLAG_ROTATE;
				}
			}

		int Temp = m_Width;
		m_Width = m_Height;
		m_Height = Temp;
		delete[] pTempData;
	}

	if(Rotation == 2 || Rotation == 3)
	{
		BrushFlipX();
		BrushFlipY();
	}

	s_lastBrushX = -1;
	s_lastBrushY = -1;
}

void CLayerTiles::Resize(int NewW, int NewH)
{
	CTile *pNewData = new CTile[NewW*NewH];
	mem_zero(pNewData, NewW*NewH*sizeof(CTile));

	// copy old data
	for(int y = 0; y < minimum(NewH, m_Height); y++)
		mem_copy(&pNewData[y*NewW], &m_pTiles[y*m_Width], minimum(m_Width, NewW)*sizeof(CTile));

	// replace old
	delete [] m_pTiles;
	m_pTiles = pNewData;
	m_Width = NewW;
	m_Height = NewH;
}

void CLayerTiles::Shift(int Direction)
{
	switch(Direction)
	{
	case 1:
		{
			// left
			for(int y = 0; y < m_Height; ++y)
				mem_move(&m_pTiles[y*m_Width], &m_pTiles[y*m_Width+1], (m_Width-1)*sizeof(CTile));
		}
		break;
	case 2:
		{
			// right
			for(int y = 0; y < m_Height; ++y)
				mem_move(&m_pTiles[y*m_Width+1], &m_pTiles[y*m_Width], (m_Width-1)*sizeof(CTile));
		}
		break;
	case 4:
		{
			// up
			for(int y = 0; y < m_Height-1; ++y)
				mem_copy(&m_pTiles[y*m_Width], &m_pTiles[(y+1)*m_Width], m_Width*sizeof(CTile));
		}
		break;
	case 8:
		{
			// down
			for(int y = m_Height-1; y > 0; --y)
				mem_copy(&m_pTiles[y*m_Width], &m_pTiles[(y-1)*m_Width], m_Width*sizeof(CTile));
		}
	}

	s_lastBrushX = -1;
	s_lastBrushY = -1;
}

void CLayerTiles::ShowInfo()
{
}


void CLayerTiles::ModifyImageIndex(INDEX_MODIFY_FUNC Func)
{
	Func(&m_Image);
}

void CLayerTiles::ModifyEnvelopeIndex(INDEX_MODIFY_FUNC Func)
{
}
