/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>

#include <engine/shared/config.h>

#include <game/client/gameclient.h>
#include <game/client/component.h>
#include <game/client/components/chat.h>

#include "controls.h"

CControls::CControls()
{
	mem_zero(&m_LastData, sizeof(m_LastData));
}

void CControls::OnReset()
{
	m_LastData.m_Direction = 0;
	m_LastData.m_Hook = 0;
	// simulate releasing the fire button
	if((m_LastData.m_Fire&1) != 0)
		m_LastData.m_Fire++;
	m_LastData.m_Fire &= INPUT_STATE_MASK;
	m_LastData.m_Jump = 0;
	m_InputData = m_LastData;

	m_InputDirectionLeft = 0;
	m_InputDirectionRight = 0;
}

void CControls::OnRelease()
{
	OnReset();
}

void CControls::OnPlayerDeath()
{
	if(!m_pClient->m_Snap.m_pGameDataRace || !(m_pClient->m_Snap.m_pGameDataRace->m_RaceFlags&RACEFLAG_KEEP_WANTED_WEAPON))
		m_LastData.m_WantedWeapon = m_InputData.m_WantedWeapon = 0;
}

struct CInputSet
{
	CControls *m_pControls;
	int *m_pVariable;
	int m_Value;
};

void CControls::OnConsoleInit()
{
}

void CControls::OnMessage(int Msg, void *pRawMsg)
{
	if(Msg == NETMSGTYPE_SV_WEAPONPICKUP)
	{
		CNetMsg_Sv_WeaponPickup *pMsg = (CNetMsg_Sv_WeaponPickup *)pRawMsg;
		m_InputData.m_WantedWeapon = pMsg->m_Weapon+1; // auto switch
	}
}

int CControls::SnapInput(int *pData)
{
	static int64 s_LastSendTime = 0;
	bool Send = false;

	// update player state
	if(m_pClient->m_pChat->IsActive())
		m_InputData.m_PlayerFlags = PLAYERFLAG_CHATTING;
	else
		m_InputData.m_PlayerFlags = 0;

	// m_InputData.m_PlayerFlags |= PLAYERFLAG_SCOREBOARD;

	if(m_LastData.m_PlayerFlags != m_InputData.m_PlayerFlags)
		Send = true;

	m_LastData.m_PlayerFlags = m_InputData.m_PlayerFlags;

	m_InputData.m_TargetX = (int)m_MousePos.x;
	m_InputData.m_TargetY = (int)m_MousePos.y;
	if(!m_InputData.m_TargetX && !m_InputData.m_TargetY)
	{
		m_InputData.m_TargetX = 1;
		m_MousePos.x = 1;
	}

	// set direction
	m_InputData.m_Direction = 0;
	if(m_InputDirectionLeft && !m_InputDirectionRight)
		m_InputData.m_Direction = -1;
	if(!m_InputDirectionLeft && m_InputDirectionRight)
		m_InputData.m_Direction = 1;

	// stress testing
	if(Config()->m_DbgStress)
	{
		float t = Client()->LocalTime();
		mem_zero(&m_InputData, sizeof(m_InputData));

		m_InputData.m_Direction = ((int)t/2)%3-1;
		m_InputData.m_Jump = ((int)t)&1;
		m_InputData.m_Fire = ((int)(t*10));
		m_InputData.m_Hook = ((int)(t*2))&1;
		m_InputData.m_WantedWeapon = ((int)t)%NUM_WEAPONS;
		m_InputData.m_TargetX = (int)(sinf(t*3)*100.0f);
		m_InputData.m_TargetY = (int)(cosf(t*3)*100.0f);
	}

	// check if we need to send input
	if(m_InputData.m_Direction != m_LastData.m_Direction) Send = true;
	else if(m_InputData.m_Jump != m_LastData.m_Jump) Send = true;
	else if(m_InputData.m_Fire != m_LastData.m_Fire) Send = true;
	else if(m_InputData.m_Hook != m_LastData.m_Hook) Send = true;
	else if(m_InputData.m_WantedWeapon != m_LastData.m_WantedWeapon) Send = true;
	else if(m_InputData.m_NextWeapon != m_LastData.m_NextWeapon) Send = true;
	else if(m_InputData.m_PrevWeapon != m_LastData.m_PrevWeapon) Send = true;

	// send at at least 10hz
	if(time_get() > s_LastSendTime + time_freq()/25)
		Send = true;

	// copy and return size
	m_LastData = m_InputData;

	if(!Send)
		return 0;

	s_LastSendTime = time_get();
	mem_copy(pData, &m_InputData, sizeof(m_InputData));
	return sizeof(m_InputData);
}

void CControls::OnRender()
{
}
