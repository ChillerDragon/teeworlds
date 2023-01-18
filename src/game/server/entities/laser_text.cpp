#include <engine/server.h>

#include "laser_text.h"

CLaserText::CLaserText(CGameWorld *pGameWorld, vec2 Pos, const char *pText)
    : CEntity(pGameWorld, CGameWorld::ENTTYPE_FLAG, Pos)
{
    m_Pos = Pos;
    m_AliveTicks = 10;

    GameWorld()->InsertEntity(this);

    for(auto &LaserRay : m_apLaserRays)
        LaserRay = nullptr;

    m_NumRays = 0;
    m_apLaserRays[m_NumRays++] = new CLaserRay(this, vec2(GetPos().x - 32, GetPos().y), vec2(GetPos().x, GetPos().y - 32));
    m_apLaserRays[m_NumRays++] = new CLaserRay(this, vec2(GetPos().x + 32, GetPos().y), vec2(GetPos().x, GetPos().y - 32));
}

CLaserRay::CLaserRay(CLaserText *pLaserText, vec2 From, vec2 To)
{
    m_pLaserText = pLaserText;
    m_From = From;
    m_To = To;
    m_ID = m_pLaserText->Server()->SnapNewID();
}

CLaserRay::~CLaserRay()
{
    m_pLaserText->Server()->SnapFreeID(m_ID);
}

CLaserText::~CLaserText()
{
    for(auto &LaserRay : m_apLaserRays)
    {
        if(LaserRay)
            delete LaserRay;
    }
}

void CLaserText::Reset()
{
    GameWorld()->DestroyEntity(this);
}

void CLaserText::Tick()
{
    if(m_AliveTicks-- < 0)
        Reset();
}

void CLaserText::Snap(int SnappingClient)
{
    if(NetworkClipped(SnappingClient))
		return;

    for(int i = 0; i < m_NumRays; i++)
    {
        CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_apLaserRays[i]->GetID(), sizeof(CNetObj_Laser)));
        if(!pObj)
            return;

        pObj->m_X = round_to_int(m_apLaserRays[i]->GetTo().x);
        pObj->m_Y = round_to_int(m_apLaserRays[i]->GetTo().y);
        pObj->m_FromX = round_to_int(m_apLaserRays[i]->GetFrom().x);
        pObj->m_FromY = round_to_int(m_apLaserRays[i]->GetFrom().y);
        pObj->m_StartTick = Server()->Tick();
    }
}
