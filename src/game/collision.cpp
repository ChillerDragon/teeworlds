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

