#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

class CCollision
{
public:

	int m_Width = 10;
	int m_Height = 10;
	int *m_pTiles;

	void Init();
	void SetTile(int x, int y, int Tile) const { m_pTiles[x + y * m_Width] = Tile; }
	int GetTile(int x, int y) const { return m_pTiles[x + y * m_Width]; }
};

#endif
