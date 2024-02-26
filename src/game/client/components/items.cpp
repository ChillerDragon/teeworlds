/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>


#include "items.h"


void CItems::RenderProjectile(const CNetObj_Projectile *pCurrent, int ItemID)
{
}

void CItems::RenderPickup(const CNetObj_Pickup *pPrev, const CNetObj_Pickup *pCurrent)
{
}

void CItems::RenderFlag(const CNetObj_Flag *pPrev, const CNetObj_Flag *pCurrent, const CNetObj_GameDataFlag *pPrevGameDataFlag, const CNetObj_GameDataFlag *pCurGameDataFlag)
{
}

void CItems::RenderLaser(const struct CNetObj_Laser *pCurrent)
{
}

void CItems::OnRender()
{
}
