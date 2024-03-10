/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SHARED_NETWORK_H
#define ENGINE_SHARED_NETWORK_H

#include <enums.h>

#include "huffman.h"

/*

CURRENT:
	packet header: 7 bytes (9 bytes for connless)
		unsigned char flags_ack;    // 6bit flags, 2bit ack
		unsigned char ack;          // 8bit ack
		unsigned char numchunks;    // 8bit chunks
		unsigned char token[4];     // 32bit token
		// ffffffaa
		// aaaaaaaa
		// NNNNNNNN
		// TTTTTTTT
		// TTTTTTTT
		// TTTTTTTT
		// TTTTTTTT

	packet header (CONNLESS):
		unsigned char flag_version;				// 6bit flags, 2bits version
		unsigned char token[4];					// 32bit token
		unsigned char responsetoken[4];			// 32bit response token

		// ffffffvv
		// TTTTTTTT
		// TTTTTTTT
		// TTTTTTTT
		// TTTTTTTT
		// RRRRRRRR
		// RRRRRRRR
		// RRRRRRRR
		// RRRRRRRR

	if the token isn't explicitely set by any means, it must be set to
	0xffffffff

	chunk header: 2-3 bytes
		unsigned char flags_size; // 2bit flags, 6 bit size
		unsigned char size_seq; // 6bit size, 2bit seq
		(unsigned char seq;) // 8bit seq, if vital flag is set
*/



typedef int (*NETFUNC_DELCLIENT)(int ClientID, const char* pReason, void *pUser);
typedef int (*NETFUNC_NEWCLIENT)(int ClientID, void *pUser);

typedef unsigned int TOKEN;

struct CNetChunk
{
	// -1 means that it's a connless packet
	// 0 on the client means the server
	int m_ClientID;
	NETADDR m_Address; // only used when cid == -1
	int m_Flags;
	int m_DataSize;
	const void *m_pData;
};

class CNetChunkHeader
{
public:
	int m_Flags;
	int m_Size;
	int m_Sequence;

	/*
	* 0.6.5 implementation of chunk header packing
	*/
	unsigned char *Pack065(unsigned char *pData);
	/*
	* 0.6.5 implementation of chunk header unpacking
	*/
	unsigned char *Unpack065(unsigned char *pData);

	/*
	* 0.7.5 implementation of chunk header packing
	*/
	unsigned char *Pack075(unsigned char *pData);
	/*
	* 0.7.5 implementation of chunk header unpacking
	*/
	unsigned char *Unpack075(unsigned char *pData);
};

class CNetPacketConstruct
{
public:
	TOKEN m_Token;
	TOKEN m_ResponseToken; // only used in connless packets
	int m_Flags;
	int m_Ack;
	int m_NumChunks;
	int m_DataSize;
	unsigned char m_aChunkData[NET_MAX_PAYLOAD];
};


class CNetBase
{
	NETSOCKET m_Socket;
	CHuffman m_Huffman;
	unsigned char m_aRequestTokenBuf[NET_TOKENREQUEST_DATASIZE];

public:
	int NetType() { return m_Socket.type; }

	void Init(NETSOCKET Socket);

	void SendControlMsg(const NETADDR *pAddr, TOKEN Token, int Ack, int ControlMsg, const void *pExtra, int ExtraSize);
	void SendControlMsgWithToken(const NETADDR *pAddr, TOKEN Token, int Ack, int ControlMsg, TOKEN MyToken, bool Extended);
	void SendPacketConnless(const NETADDR *pAddr, TOKEN Token, TOKEN ResponseToken, const void *pData, int DataSize);
	void SendPacket(const NETADDR *pAddr, CNetPacketConstruct *pPacket);
	int UnpackPacket(NETADDR *pAddr, unsigned char *pBuffer, CNetPacketConstruct *pPacket);
};

typedef void(*FSendCallback)(int TrackID, void *pUser);
struct CSendCBData
{
	FSendCallback m_pfnCallback;
	void *m_pCallbackUser;
	int m_TrackID;
};

class CNetConnection
{
	// TODO: is this needed because this needs to be aware of
	// the ack sequencing number and is also responible for updating
	// that. this should be fixed.
	friend class CNetRecvUnpacker;
private:
	unsigned short m_Sequence;
	unsigned short m_Ack;
	unsigned short m_PeerAck;
	unsigned m_State;

	int m_RemoteClosed;
	bool m_BlockCloseMsg;

	int64 m_LastUpdateTime;
	int64 m_LastRecvTime;
	int64 m_LastSendTime;

	CNetPacketConstruct m_Construct;

	TOKEN m_Token;
	TOKEN m_PeerToken;
	NETADDR m_PeerAddr;

	NETSTATS m_Stats;
	CNetBase *m_pNetBase;

	//
	void Reset();
	void ResetStats();
	int QueueChunkEx(int Flags, int DataSize, const void *pData, int Sequence);
	void SendControl(int ControlMsg, const void *pExtra, int ExtraSize);
	void SendControlWithToken(int ControlMsg);

public:
	void Init(CNetBase *pNetBase, bool BlockCloseMsg);
	int Connect(NETADDR *pAddr);
	void Disconnect(const char *pReason);

	void SetToken(TOKEN Token);

	TOKEN Token() const { return m_Token; }
	TOKEN PeerToken() const { return m_PeerToken; }

	int Flush();

	int Feed(CNetPacketConstruct *pPacket, NETADDR *pAddr);
	int QueueChunk(int Flags, int DataSize, const void *pData);
	void SendPacketConnless(const char *pData, int DataSize);

	int State() const { return m_State; }
	const NETADDR *PeerAddress() const { return &m_PeerAddr; }
};

class CConsoleNetConnection
{
private:
	int m_State;

	NETADDR m_PeerAddr;
	NETSOCKET m_Socket;

	char m_aBuffer[NET_MAX_PACKETSIZE];
	int m_BufferOffset;

	bool m_LineEndingDetected;
	char m_aLineEnding[3];

public:
	void Init(NETSOCKET Socket, const NETADDR *pAddr);
	void Disconnect(const char *pReason);

	int State() const { return m_State; }
	const NETADDR *PeerAddress() const { return &m_PeerAddr; }

	void Reset();
	int Send(const char *pLine);
	int Recv(char *pLine, int MaxLength);
};

class CNetRecvUnpacker
{
	bool m_Valid;

public:
	NETADDR m_Addr;
	CNetConnection *m_pConnection;
	int m_CurrentChunk;
	int m_ClientID;
	CNetPacketConstruct m_Data;
	unsigned char m_aBuffer[NET_MAX_PACKETSIZE];

	CNetRecvUnpacker() { Clear(); }
	bool IsActive() { return m_Valid; }
	void Clear();
	void Start(const NETADDR *pAddr, CNetConnection *pConnection, int ClientID);
	int FetchChunk(CNetChunk *pChunk);
};

// server side
class CNetServer : public CNetBase
{
	struct CSlot
	{
	public:
		CNetConnection m_Connection;
	};

	CSlot m_aSlots[NET_MAX_CLIENTS];
	int m_NumClients;
	int m_MaxClients;
	int m_MaxClientsPerIP;

	NETFUNC_NEWCLIENT m_pfnNewClient;
	NETFUNC_DELCLIENT m_pfnDelClient;
	void *m_UserPtr;

	CNetRecvUnpacker m_RecvUnpacker;

public:
	// the token parameter is only used for connless packets
	int Recv(CNetChunk *pChunk, TOKEN *pResponseToken = 0);
	int Send(CNetChunk *pChunk, TOKEN Token = NET_TOKEN_NONE);


	// status requests
	const NETADDR *ClientAddr(int ClientID) const { return m_aSlots[ClientID].m_Connection.PeerAddress(); }
};


// client side
class CNetClient : public CNetBase
{
	CNetConnection m_Connection;
	CNetRecvUnpacker m_RecvUnpacker;

	int m_Flags;

public:

	// connection state
	int Disconnect(const char *Reason);
	int Connect(NETADDR *Addr);

	// communication
	int Recv(CNetChunk *pChunk, TOKEN *pResponseToken = 0);
	int Send(CNetChunk *pChunk, TOKEN Token = NET_TOKEN_NONE, CSendCBData *pCallbackData = 0);
	void PurgeStoredPacket(int TrackID);

	// pumping
	int Flush();

	// error and state
	int State() const;
	bool GotProblems() const;
};

#endif
