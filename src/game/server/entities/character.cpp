/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <generated/server_data.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/player.h>

#include "character.h"
#include "laser.h"
#include "projectile.h"

//input count
struct CInputCount
{
	int m_Presses;
	int m_Releases;
};

CInputCount CountInput(int Prev, int Cur)
{
	CInputCount c = {0, 0};
	Prev &= INPUT_STATE_MASK;
	Cur &= INPUT_STATE_MASK;
	int i = Prev;

	while(i != Cur)
	{
		i = (i+1)&INPUT_STATE_MASK;
		if(i&1)
			c.m_Presses++;
		else
			c.m_Releases++;
	}

	return c;
}


MACRO_ALLOC_POOL_ID_IMPL(CCharacter, MAX_CLIENTS)

// Character, "physical" player's part
CCharacter::CCharacter()
{
	m_Health = 0;
	m_Armor = 0;
	m_TriggeredEvents = 0;
}

void CCharacter::Reset()
{
	Destroy();
}

bool CCharacter::Spawn(CPlayer *pPlayer, vec2 Pos)
{
	m_EmoteStop = -1;
	m_LastAction = -1;
	m_LastNoAmmoSound = -1;
	m_ActiveWeapon = WEAPON_GUN;
	m_LastWeapon = WEAPON_HAMMER;
	m_QueuedWeapon = -1;

	m_pPlayer = pPlayer;
	m_Pos = Pos;

	m_ReckoningTick = 0;

	m_Alive = true;
	return true;
}

void CCharacter::Destroy()
{
	m_Alive = false;
}

void CCharacter::SetWeapon(int W)
{
}

bool CCharacter::IsGrounded()
{
	return false;
}


void CCharacter::HandleNinja()
{
}


void CCharacter::DoWeaponSwitch()
{
}

void CCharacter::HandleWeaponSwitch()
{
}

void CCharacter::FireWeapon()
{
}

void CCharacter::HandleWeapons()
{
}

bool CCharacter::GiveWeapon(int Weapon, int Ammo)
{
	return false;
}

void CCharacter::GiveNinja()
{
}

void CCharacter::SetEmote(int Emote, int Tick)
{
}

void CCharacter::OnPredictedInput(CNetObj_PlayerInput *pNewInput)
{
}

void CCharacter::OnDirectInput(CNetObj_PlayerInput *pNewInput)
{
}

void CCharacter::ResetInput()
{
}

void CCharacter::Tick()
{
}

void CCharacter::TickDefered()
{
}

void CCharacter::TickPaused()
{
}

bool CCharacter::IncreaseHealth(int Amount)
{
	return true;
}

bool CCharacter::IncreaseArmor(int Amount)
{
	return true;
}

void CCharacter::Die(int Killer, int Weapon)
{
}

bool CCharacter::TakeDamage(vec2 Force, vec2 Source, int Dmg, int From, int Weapon)
{
	return true;
}

void CCharacter::Snap(int SnappingClient)
{
}

void CCharacter::PostSnap()
{
	m_TriggeredEvents = 0;
}
