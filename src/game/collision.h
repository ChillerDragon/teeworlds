#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

class CCollision
{
public:

	int m_Width = 10;
	int m_Height = 10;
	int *m_pTiles;

	void Init();
	void SetTile(int x, int y, int Tile) const;
	int GetTile(int x, int y) const;
};

#endif
