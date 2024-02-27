/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>

#include "character.h"
#include "flag.h"

CFlag::CFlag(int Team, vec2 StandPos)
{
	m_Team = Team;
	m_StandPos = StandPos;

	Reset();
}

void CFlag::Reset()
{
	m_pCarrier = 0;
	m_AtStand = true;
	m_Pos = m_StandPos;
	m_Vel = vec2(0, 0);
	m_GrabTick = 0;
}

void CFlag::Grab(CCharacter *pChar)
{
}

void CFlag::Drop()
{
}

void CFlag::TickDefered()
{
}

void CFlag::TickPaused()
{
	m_DropTick++;
	if(m_GrabTick)
		m_GrabTick++;
}

void CFlag::Snap(int SnappingClient)
{
}
