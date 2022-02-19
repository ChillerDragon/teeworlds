/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_MYMOD_H
#define GAME_SERVER_GAMEMODES_MYMOD_H
#include <game/server/gamecontroller.h>

// you can subclass GAMECONTROLLER_CTF, GAMECONTROLLER_TDM etc if you want
// todo a modification with their base as well.
class CGameControllerMymod : public IGameController
{
public:
	CGameControllerMymod(class CGameContext *pGameServer);
	virtual void Tick();
	virtual void OnCharacterSpawn(class CCharacter *pChr);
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void OnPlayerDisconnect(class CPlayer *pPlayer);
	// add more virtual functions here if you wish
};
#endif
