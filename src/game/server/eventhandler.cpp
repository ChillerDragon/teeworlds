/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include "eventhandler.h"
#include "gamecontext.h"
#include "player.h"

void *CEventHandler::Create(int Type, int Size, int64 Mask)
{
	if(m_NumEvents == MAX_EVENTS)
		return 0;
	if(m_CurrentOffset+Size >= MAX_DATASIZE)
		return 0;

	void *p = &m_aData[m_CurrentOffset];
	m_aOffsets[m_NumEvents] = m_CurrentOffset;
	m_aTypes[m_NumEvents] = Type;
	m_aSizes[m_NumEvents] = Size;
	m_aClientMasks[m_NumEvents] = Mask;
	m_CurrentOffset += Size;
	m_NumEvents++;
	return p;
}

void CEventHandler::Snap(int SnappingClient)
{
	for(int i = 0; i < m_NumEvents; i++)
	{
		if(SnappingClient == -1 || CmaskIsSet(m_aClientMasks[i], SnappingClient))
		{
			CNetEvent_Common *ev = (CNetEvent_Common *)&m_aData[m_aOffsets[i]];
			void *d = GameServer()->Server()->SnapNewItem(m_aTypes[i], i, m_aSizes[i]);
			if(d)
				mem_copy(d, &m_aData[m_aOffsets[i]], m_aSizes[i]);
		}
	}
}
