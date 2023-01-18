/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_LASER_TEXT_H
#define GAME_SERVER_ENTITIES_LASER_TEXT_H

#include <game/server/entity.h>

class CLaserText;

class CLaserRay
{
    CLaserText *m_pLaserText;
    vec2 m_From;
    vec2 m_To;
    int m_ID;

public:
    CLaserRay(CLaserText *pLaserText, vec2 From, vec2 To);
    ~CLaserRay();
    int GetID() { return m_ID; }
    vec2 GetFrom() { return m_From; }
    vec2 GetTo() { return m_To; }
};

class CLaserText : public CEntity
{
    vec2 m_Pos;
    int64_t m_AliveTicks;

    CLaserRay *m_apLaserRays[16];
    int m_NumRays;
public:
	/* Constructor */
	CLaserText(CGameWorld *pGameWorld, vec2 Pos, const char *pText);
    ~CLaserText();

	/* CEntity functions */
	virtual void Reset();
	virtual void Snap(int SnappingClient);
	virtual void Tick();
};

#endif
