/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/ui.h>
#include <game/client/render.h>
#include "damageind.h"

CDamageInd::CDamageInd()
{
	m_NumItems = 0;
}

CDamageInd::CItem *CDamageInd::CreateItem()
{
	return 0;
}

void CDamageInd::DestroyItem(CDamageInd::CItem *pItem)
{
}

void CDamageInd::Create(vec2 Pos, vec2 Dir)
{
}

void CDamageInd::OnRender()
{
}

void CDamageInd::OnReset()
{
}
