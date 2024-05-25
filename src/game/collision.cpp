#include <cstdlib>
#include <game/collision.h>

void CCollision::Init()
{
	m_pTiles = (int *)malloc(sizeof(int) * m_Width * m_Height);

	for(int x = 0; x < m_Width; x++)
	{
		for(int y = 0; y < m_Height; y++)
		{
			SetTile(x, y, 22);
		}
	}
	SetTile(1, 1, 11);
}

void CCollision::SetTile(int x, int y, int Tile) const { m_pTiles[x + y * m_Width] = Tile; }

int CCollision::GetTile(int x, int y) const { return m_pTiles[x + y * m_Width]; }
