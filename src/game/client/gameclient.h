/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_GAMECLIENT_H
#define GAME_CLIENT_GAMECLIENT_H

#include <base/vmath.h>
#include <engine/client.h>
#include "render.h"

#include <generated/protocol.h>

class CGameClient : public IGameClient
{
	class CStack
	{
	public:
		enum
		{
			MAX_COMPONENTS = 64,
		};

		CStack();
		void Add(class CComponent *pComponent);

		class CComponent *m_paComponents[MAX_COMPONENTS];
		int m_Num;
	};

	CStack m_All;
	CStack m_Input;
	CNetObjHandler m_NetObjHandler;

	class IEngine *m_pEngine;
	class IClient *m_pClient;
	class CConfig *m_pConfig;
	class IStorage *m_pStorage;

	void ProcessEvents();
	void ProcessTriggeredEvents(int Events, vec2 Pos);
	void UpdatePositions();

	int m_PredictedTick;
	int m_LastNewPredictedTick;

	int m_LastGameStartTick;
	int m_LastFlagCarrierRed;
	int m_LastFlagCarrierBlue;

	void LoadFonts();

public:
	IKernel *Kernel() { return IInterface::Kernel(); }
	IEngine *Engine() const { return m_pEngine; }
	class IClient *Client() const { return m_pClient; }
	class IStorage *Storage() const { return m_pStorage; }
	class CConfig *Config() const { return m_pConfig; }

	const char *NetobjFailedOn() { return m_NetObjHandler.FailedObjOn(); }
	int NetobjNumFailures() { return m_NetObjHandler.NumObjFailures(); }
	const char *NetmsgFailedOn() { return m_NetObjHandler.FailedMsgOn(); }

	bool m_SuppressEvents;

	enum
	{
		SERVERMODE_PURE=0,
		SERVERMODE_MOD,
		SERVERMODE_PUREMOD,
	};
	int m_ServerMode;

	int m_DemoSpecMode;
	int m_DemoSpecID;

	vec2 m_LocalCharacterPos;

	vec2 GetCharPos(int ClientID, bool Predicted = false) const;

	// ---

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
		bool m_ChatIgnore;
		bool m_Friend;

		void UpdateRenderInfo(CGameClient *pGameClient, int ClientID, bool UpdateSkinInfo);
		void Reset(CGameClient *pGameClient, int CLientID);
	};

	CClientData m_aClients[MAX_CLIENTS];
	int m_LocalClientID;
	int m_TeamCooldownTick;
	float m_TeamChangeTime;
	bool m_IsXmasDay;
	float m_LastSkinChangeTime;
	int m_IdentityState;
	bool m_IsEasterDay;
	bool m_InitComplete;

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

	struct CServerSettings
	{
		bool m_KickVote;
		int m_KickMin;
		bool m_SpecVote;
		bool m_TeamLock;
		bool m_TeamBalance;
		int m_PlayerSlots;
	} m_ServerSettings;

	void OnReset();

	// hooks
	virtual void OnConnected();
	virtual void OnRender();
	virtual void OnUpdate();
	virtual void OnRelease();
	virtual void OnInit();
	virtual void OnConsoleInit();
	virtual void OnStateChange(int NewState, int OldState);
	virtual void OnMessage(int MsgId, CUnpacker *pUnpacker);
	virtual void OnNewSnapshot();
	virtual void OnDemoRecSnap();
	virtual int OnSnapInput(int *pData);
	virtual void OnShutdown();
	virtual void OnEnterGame();
	virtual void OnRconLine(const char *pLine);
	virtual void OnGameOver();
	virtual void OnStartGame();

	virtual const char *GetItemName(int Type) const;
	virtual const char *Version() const;
	virtual const char *NetVersion() const;
	virtual const char *NetVersionHashUsed() const;
	virtual const char *NetVersionHashReal() const;
	virtual int ClientVersion() const;
	void GetPlayerLabel(char* aBuf, int BufferSize, int ClientID, const char* ClientName);
	void StartRendering();

	bool IsXmas() const;
	bool IsEaster() const;
	int RacePrecision() const { return m_Snap.m_pGameDataRace ? m_Snap.m_pGameDataRace->m_Precision : 3; }
	bool IsWorldPaused() const { return m_Snap.m_pGameData && (m_Snap.m_pGameData->m_GameStateFlags&(GAMESTATEFLAG_PAUSED|GAMESTATEFLAG_ROUNDOVER|GAMESTATEFLAG_GAMEOVER)); }
	bool IsDemoPlaybackPaused() const;
	float GetAnimationPlaybackSpeed() const;

	//
	void DoEnterMessage(const char *pName, int ClientID, int Team);
	void DoLeaveMessage(const char *pName, int ClientID, const char *pReason);
	void DoTeamChangeMessage(const char *pName, int ClientID, int Team);

	int GetClientID(const char *pName);

	// actions
	// TODO: move these
	void SendSwitchTeam(int Team);
	void SendStartInfo();
	void SendKill();
	void SendReadyChange();
	void SendSkinChange();

	// pointers to all systems
	class CBroadcast *m_pBroadcast;
	class CChat *m_pChat;
	class CControls *m_pControls;
	class CMotd *m_pMotd;
	class CVoting *m_pVoting;
	class CScoreboard *m_pScoreboard;
	class CItems *m_pItems;
};


void FormatTime(char *pBuf, int Size, int Time, int Precision);
void FormatTimeDiff(char *pBuf, int Size, int Time, int Precision, bool ForceSign = true);

const char *Localize(const char *pStr, const char *pContext="")
GNUC_ATTRIBUTE((format_arg(1)));

#endif
