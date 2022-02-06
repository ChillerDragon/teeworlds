/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/entities/character.h>
#include <engine/server.h>

#include "mymod.h"

CGameControllerMymod::CGameControllerMymod(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	// Exchange this to a string that identifies your game mode.
	// DM, TDM and CTF are reserved for teeworlds original modes.
	m_pGameType = "mymod";

	// m_GameFlags = GAMEFLAG_TEAMS; // GAMEFLAG_TEAMS makes it a two-team gamemode
}

void CGameControllerMymod::Tick()
{
	// this is the main part of the gamemode, this function is run every tick

	IGameController::Tick();
}

void CGameControllerMymod::OnCharacterSpawn(class CCharacter *pChr)
{
	IGameController::OnCharacterSpawn(pChr);

	dbg_msg("mymod", "character spawned!!");
}

int CGameControllerMymod::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	char aBuf[128];
	if(pKiller && pVictim)
	{
		str_format(aBuf,
			sizeof(aBuf),
			"'%s' killed '%s'",
			Server()->ClientName(pKiller->GetCID()),
			Server()->ClientName(pVictim->GetPlayer()->GetCID())
			);


		GameServer()->SendChat(-1, CHAT_ALL, -1, aBuf);
	}

	if(pKiller)
	{
		pKiller->m_AccountData.m_Xp++;
		if(pKiller->m_AccountData.m_Xp % 2 == 0)
		{
			pKiller->m_AccountData.m_Level++;
			str_format(aBuf, sizeof(aBuf), "Level up! You are now level %d!", pKiller->m_AccountData.m_Level);
			GameServer()->SendChat(-1, CHAT_ALL, pKiller->GetCID(), aBuf);
		}
	}


	return 0;
}
