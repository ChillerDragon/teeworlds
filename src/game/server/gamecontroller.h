/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMECONTROLLER_H
#define GAME_SERVER_GAMECONTROLLER_H

#include <base/vmath.h>

#include <generated/protocol.h>

/*
	Class: Game Controller
		Controls the main game logic. Keeping track of team and player score,
		winning conditions and specific game logic.
*/
class IGameController
{
	class CGameContext *m_pGameServer;
	class IServer *m_pServer;

	// activity
	bool GetPlayersReadyState(int WithoutID = -1);
	void SetPlayersReadyState(bool ReadyState);
	void CheckReadyStates(int WithoutID = -1);

protected:
	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const { return m_pServer; }
	void UpdateGameInfo(int ClientID);

public:
	IGameController(class CGameContext *pGameServer);
	virtual ~IGameController() {}
	void OnPlayerCommand(class CPlayer *pPlayer, const char *pCommandName, const char *pCommandArgs);
	void OnReset();
	virtual void Snap(int SnappingClient);
	void SendTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg=true);
};

#endif
