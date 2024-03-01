/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_GAMECLIENT_H
#define GAME_CLIENT_GAMECLIENT_H

#include <base/vmath.h>
#include <engine/client.h>

#include <generated/protocol.h>

class CGameClient : public IGameClient
{
	CNetObjHandler m_NetObjHandler;

	class IClient *m_pClient;

	void ProcessEvents();
	void ProcessTriggeredEvents(int Events, vec2 Pos);
	void UpdatePositions();

	int m_PredictedTick;
	int m_LastNewPredictedTick;

public:
	class IClient *Client() const { return m_pClient; }

	const char *NetobjFailedOn() { return m_NetObjHandler.FailedObjOn(); }
	int NetobjNumFailures() { return m_NetObjHandler.NumObjFailures(); }
	const char *NetmsgFailedOn() { return m_NetObjHandler.FailedMsgOn(); }

	struct CPlayerInfoItem
	{
		const CNetObj_PlayerInfo *m_pPlayerInfo;
		int m_ClientID;
	};

	// snap pointers
	struct CSnapState
	{
		const CNetObj_Character *m_pLocalCharacter;
		const CNetObj_Character *m_pLocalPrevCharacter;
		const CNetObj_PlayerInfo *m_pLocalInfo;
		const CNetObj_SpectatorInfo *m_pSpectatorInfo;
		const CNetObj_SpectatorInfo *m_pPrevSpectatorInfo;
		const CNetObj_Flag *m_paFlags[2];
		const CNetObj_GameData *m_pGameData;
		const CNetObj_GameDataTeam *m_pGameDataTeam;
		const CNetObj_GameDataFlag *m_pGameDataFlag;
		const CNetObj_GameDataRace *m_pGameDataRace;
		int m_GameDataFlagSnapID;

		int m_NotReadyCount;
		int m_AliveCount[NUM_TEAMS];

		const CNetObj_PlayerInfo *m_paPlayerInfos[MAX_CLIENTS];
		const CNetObj_PlayerInfoRace *m_paPlayerInfosRace[MAX_CLIENTS];
		CPlayerInfoItem m_aInfoByScore[MAX_CLIENTS];

		// spectate data
		struct CSpectateInfo
		{
			bool m_Active;
			int m_SpecMode;
			int m_SpectatorID;
			bool m_UsePosition;
			vec2 m_Position;
		} m_SpecInfo;

		//
		struct CCharacterInfo
		{
			bool m_Active;

			// snapshots
			CNetObj_Character m_Prev;
			CNetObj_Character m_Cur;

			// interpolated position
			vec2 m_Position;
		};

		CCharacterInfo m_aCharacters[MAX_CLIENTS];
	};

	CSnapState m_Snap;

	// client data
	struct CClientData
	{
		char m_aName[MAX_NAME_ARRAY_SIZE];
		char m_aClan[MAX_CLAN_ARRAY_SIZE];
		int m_Country;
		char m_aaSkinPartNames[NUM_SKINPARTS][MAX_SKIN_ARRAY_SIZE];
		int m_aUseCustomColors[NUM_SKINPARTS];
		int m_aSkinPartColors[NUM_SKINPARTS];
		int m_SkinPartIDs[NUM_SKINPARTS];
		int m_Team;
		int m_Emoticon;
		int m_EmoticonStart;


		CNetObj_Character m_Evolved;

		float m_Angle;
		bool m_Active;

		void Reset(CGameClient *pGameClient, int CLientID);
	};

	CClientData m_aClients[MAX_CLIENTS];
	int m_LocalClientID;
	int m_TeamCooldownTick;
	float m_TeamChangeTime;
	float m_LastSkinChangeTime;
	int m_IdentityState;

	struct CGameInfo
	{
		int m_GameFlags;
		int m_ScoreLimit;
		int m_TimeLimit;
		int m_MatchNum;
		int m_MatchCurrent;

		int m_NumPlayers;
		int m_aTeamSize[NUM_TEAMS];
	};

	CGameInfo m_GameInfo;

	void OnReset();

	// hooks
	virtual void OnMessage(int MsgId, CUnpacker *pUnpacker);
	virtual void OnNewSnapshot();
	virtual int OnSnapInput(int *pData);
	virtual void OnRconLine(const char *pLine);

	virtual const char *GetItemName(int Type) const;
	virtual const char *Version() const;
	virtual const char *NetVersion() const;
	virtual const char *NetVersionHashUsed() const;
	virtual const char *NetVersionHashReal() const;
	virtual int ClientVersion() const;

	int SnapInput(int *pData);
	CNetObj_PlayerInput m_InputData;

	void SendSwitchTeam(int Team);
	void SendStartInfo();
	void SendKill();
	void SendReadyChange();
	void SendSkinChange();
	void SendEmoticon(int Emoticon);
	void SendSpectate(int SpecMode, int SpectatorID);
	void SendCallvote(const char *pType, const char *pValue, const char *pReason, bool ForceVote);
	void SendVote(int Choice);
	void SendChat(int Mode, const char *pLine);
};


#endif
