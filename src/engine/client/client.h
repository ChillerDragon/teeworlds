/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_CLIENT_H
#define ENGINE_CLIENT_CLIENT_H

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



class CClient : public IClient
{
	// needed interfaces
	IGameClient *m_pGameClient;

	enum
	{
		NUM_SNAPSHOT_TYPES=2,
		PREDICTION_MARGIN=1000/50/2, // magic network prediction value
	};

	class CNetClient m_NetClient;
	class CNetClient m_ContactClient;

	char m_aServerPassword[128];

	unsigned m_SnapshotParts;
	int64 m_LocalStartTime;

	int m_WindowMustRefocus;
	int m_SnapCrcErrors;

	int m_AckGameTick;
	int m_CurrentRecvTick;
	int m_RconAuthed;
	int m_UseTempRconCommands;

	// version-checking
	char m_aVersionStr[10];

	char m_aCmdConnect[256];

	// map download
	char m_aMapdownloadFilename[IO_MAX_PATH_LENGTH];
	char m_aMapdownloadFilenameTemp[IO_MAX_PATH_LENGTH];
	char m_aMapdownloadName[IO_MAX_PATH_LENGTH];
	int m_MapdownloadChunk;
	int m_MapdownloadChunkNum;
	int m_MapDownloadChunkSize;
	int m_MapdownloadCrc;
	int m_MapdownloadAmount;
	int m_MapdownloadTotalsize;

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
	IGameClient *GameClient() { return m_pGameClient; }

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

	void SendInput();


	const void *SnapGetItem(int SnapID, int Index, CSnapItem *pItem) const;
	void SnapInvalidateItem(int SnapID, int Index);
	const void *SnapFindItem(int SnapID, int Type, int ID) const;
	int SnapNumItems(int SnapID) const;
	void *SnapNewItem(int Type, int ID, int Size);
	void SnapSetStaticsize(int ItemType, int Size);

	void ProcessConnlessPacket(CNetChunk *pPacket);
	void ProcessServerPacket(CNetChunk *pPacket);

	virtual const char *MapDownloadName() const { return m_aMapdownloadName; }
	virtual int MapDownloadAmount() const { return m_MapdownloadAmount; }
	virtual int MapDownloadTotalsize() const { return m_MapdownloadTotalsize; }

	void PumpNetwork();
};
#endif
