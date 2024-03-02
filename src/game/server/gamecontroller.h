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

	// balancing
	enum
	{
		TBALANCE_CHECK=-2,
		TBALANCE_OK,
	};
	int m_aTeamSize[NUM_TEAMS];

protected:
	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const { return m_pServer; }

	// game
	int m_GameStartTick;
	int m_MatchCount;
	int m_RoundCount;
	int m_SuddenDeath;
	int m_aTeamscore[NUM_TEAMS];

	// info
	int m_GameFlags;
	const char *m_pGameType;
	struct CGameInfo
	{
		int m_MatchCurrent;
		int m_MatchNum;
		int m_ScoreLimit;
		int m_TimeLimit;
	} m_GameInfo;

	void UpdateGameInfo(int ClientID);

public:
	IGameController(class CGameContext *pGameServer);
	virtual ~IGameController() {}

	void OnPlayerCommand(class CPlayer *pPlayer, const char *pCommandName, const char *pCommandArgs);

	void OnReset();

	// game
	enum
	{
		TIMER_INFINITE = -1,
		TIMER_END = 10,
	};

	virtual void Snap(int SnappingClient);

	// info
	void CheckGameInfo();
	const char *GetGameType() const { return "dm"; }


	void SendTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg=true);

	int GetRealPlayerNum() const { return m_aTeamSize[TEAM_RED]+m_aTeamSize[TEAM_BLUE]; }

};

#endif
