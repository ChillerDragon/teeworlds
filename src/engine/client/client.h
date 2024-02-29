/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_CLIENT_H
#define ENGINE_CLIENT_CLIENT_H

#include <base/hash.h>

class CGraph
{
	enum
	{
		MAX_VALUES=128,
	};

	float m_Min, m_Max;
	float m_MinRange, m_MaxRange;
	float m_aValues[MAX_VALUES];
	float m_aColors[MAX_VALUES][3];
	int m_Index;

public:
	void Init(float Min, float Max);

	void Scale();
	void Add(float v, float r, float g, float b);
};


class CSmoothTime
{
	int64 m_Snap;
	int64 m_Current;
	int64 m_Target;

	CGraph m_Graph;

	int m_SpikeCounter;
	int m_BadnessScore; // ranges between -100 (perfect) and MAX_INT

	float m_aAdjustSpeed[2]; // 0 = down, 1 = up
public:
	void Init(int64 Target);
	void SetAdjustSpeed(int Direction, float Value);

	int64 Get(int64 Now);
	inline int GetStabilityScore() const { return m_BadnessScore; }

	void UpdateInt(int64 Target);
	void Update(CGraph *pGraph, int64 Target, int TimeLeft, int AdjustDirection);
};


class CClient : public IClient
{
	// needed interfaces
	IEngine *m_pEngine;
	IGameClient *m_pGameClient;
	IConfigManager *m_pConfigManager;
	CConfig *m_pConfig;

	enum
	{
		NUM_SNAPSHOT_TYPES=2,
		PREDICTION_MARGIN=1000/50/2, // magic network prediction value
	};

	class CNetClient m_NetClient;
	class CNetClient m_ContactClient;

	char m_aServerAddressStr[256];
	char m_aServerPassword[128];

	unsigned m_SnapshotParts;
	int64 m_LocalStartTime;

	int64 m_LastRenderTime;
	int64 m_LastCpuTime;
	float m_LastAvgCpuFrameTime;
	float m_RenderFrameTimeLow;
	float m_RenderFrameTimeHigh;
	int m_RenderFrames;

	NETADDR m_ServerAddress;
	int m_WindowMustRefocus;
	int m_SnapCrcErrors;

	int m_AckGameTick;
	int m_CurrentRecvTick;
	int m_RconAuthed;
	int m_UseTempRconCommands;

	// version-checking
	char m_aVersionStr[10];

	// pinging
	int64 m_PingStartTime;

	//
	char m_aCurrentMap[256];
	char m_aCurrentMapPath[IO_MAX_PATH_LENGTH];
	SHA256_DIGEST m_CurrentMapSha256;
	unsigned m_CurrentMapCrc;

	//
	char m_aCmdConnect[256];

	// map download
	char m_aMapdownloadFilename[IO_MAX_PATH_LENGTH];
	char m_aMapdownloadFilenameTemp[IO_MAX_PATH_LENGTH];
	char m_aMapdownloadName[IO_MAX_PATH_LENGTH];
	IOHANDLE m_MapdownloadFileTemp;
	int m_MapdownloadChunk;
	int m_MapdownloadChunkNum;
	int m_MapDownloadChunkSize;
	SHA256_DIGEST m_MapdownloadSha256;
	bool m_MapdownloadSha256Present;
	int m_MapdownloadCrc;
	int m_MapdownloadAmount;
	int m_MapdownloadTotalsize;

	// time
	CSmoothTime m_GameTime;
	CSmoothTime m_PredictedTime;

	// input
	struct // TODO: handle input better
	{
		int m_aData[MAX_INPUT_SIZE]; // the input data
		int m_Tick; // the tick that the input is for
		int64 m_PredictedTime; // prediction latency when we sent this input
		int64 m_Time;
	} m_aInputs[200];

	int m_CurrentInput;

	// graphs
	CGraph m_InputtimeMarginGraph;
	CGraph m_GametimeMarginGraph;
	CGraph m_FpsGraph;

	// the game snapshots are modifiable by the game
	class CSnapshotStorage m_SnapshotStorage;
	CSnapshotStorage::CHolder *m_aSnapshots[NUM_SNAPSHOT_TYPES];

	int m_ReceivedSnapshots;
	char m_aSnapshotIncomingData[CSnapshot::MAX_SIZE];

	class CSnapshotStorage::CHolder m_aDemorecSnapshotHolders[NUM_SNAPSHOT_TYPES];
	char *m_aDemorecSnapshotData[NUM_SNAPSHOT_TYPES][2][CSnapshot::MAX_SIZE];
	class CSnapshotBuilder m_DemoRecSnapshotBuilder;

	class CSnapshotDelta m_SnapshotDelta;

	int64 TickStartTime(int Tick);

public:
	IEngine *Engine() { return m_pEngine; }
	IGameClient *GameClient() { return m_pGameClient; }
	IConfigManager *ConfigManager() { return m_pConfigManager; }
	CConfig *Config() { return m_pConfig; }

	CClient();

	// ----- send functions -----
	virtual int SendMsg(CMsgPacker *pMsg, int Flags);

	void SendInfo();
	void SendEnterGame();
	void SendReady();

	virtual bool RconAuthed() const { return m_RconAuthed != 0; }
	virtual bool UseTempRconCommands() const { return m_UseTempRconCommands != 0; }
	void RconAuth(const char *pName, const char *pPassword);
	virtual void Rcon(const char *pCmd);

	virtual bool ConnectionProblems() const;
	virtual int GetInputtimeMarginStabilityScore();

	void SendInput();

	// TODO: OPT: do this alot smarter!
	virtual const int *GetInput(int Tick) const;

	const char *LatestVersion() const;
	void VersionUpdate();

	// ------ state handling -----
	void SetState(int s);

	// called when the map is loaded and we should init for a new round
	void OnEnterGame();
	virtual void EnterGame();
	void OnClientOnline();

	virtual void Connect(const char *pAddress);
	void DisconnectWithReason(const char *pReason);
	virtual void Disconnect();
	const char *ServerAddress() const { return m_aServerAddressStr; }


	// ---

	const void *SnapGetItem(int SnapID, int Index, CSnapItem *pItem) const;
	void SnapInvalidateItem(int SnapID, int Index);
	const void *SnapFindItem(int SnapID, int Type, int ID) const;
	int SnapNumItems(int SnapID) const;
	void *SnapNewItem(int Type, int ID, int Size);
	void SnapSetStaticsize(int ItemType, int Size);

	void Render();
	void DebugRender();

	virtual void Quit();

	virtual const char *ErrorString() const;

	const char *LoadMap(const char *pName, const char *pFilename, const SHA256_DIGEST *pWantedSha256, unsigned WantedCrc);
	const char *LoadMapSearch(const char *pMapName, const SHA256_DIGEST *pWantedSha256, int WantedCrc);

	void ProcessConnlessPacket(CNetChunk *pPacket);
	void ProcessServerPacket(CNetChunk *pPacket);

	const char *GetCurrentMapName() const { return m_aCurrentMap; }
	const char *GetCurrentMapPath() const { return m_aCurrentMapPath; }
	virtual const char *MapDownloadName() const { return m_aMapdownloadName; }
	virtual int MapDownloadAmount() const { return m_MapdownloadAmount; }
	virtual int MapDownloadTotalsize() const { return m_MapdownloadTotalsize; }

	void PumpNetwork();

	void Update();

	void RegisterInterfaces();
	void InitInterfaces();

	void Run();

	void ConnectOnStart(const char *pAddress);

	// chillers verbose network printer
	void PrintSnapshot(int Msg, CUnpacker &Unpacker);
};
#endif
