/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>
#include "network.h"


void CNetConnection::ResetStats()
{
	mem_zero(&m_Stats, sizeof(m_Stats));
}

void CNetConnection::Reset()
{
	m_Sequence = 0;
	m_Ack = 0;
	m_RemoteClosed = 0;

	m_LastSendTime = 0;
	m_LastRecvTime = 0;
	m_LastUpdateTime = 0;
	m_Token = NET_TOKEN_NONE;
	m_PeerToken = NET_TOKEN_NONE;
	mem_zero(&m_PeerAddr, sizeof(m_PeerAddr));

	mem_zero(&m_Construct, sizeof(m_Construct));
}

void CNetConnection::SetToken(TOKEN Token)
{
	m_Token = Token;
}

void CNetConnection::Init(CNetBase *pNetBase, bool BlockCloseMsg)
{
	Reset();
	ResetStats();

	m_pNetBase = pNetBase;
	m_BlockCloseMsg = BlockCloseMsg;
}

int CNetConnection::Flush()
{
	int NumChunks = m_Construct.m_NumChunks;
	if(!NumChunks && !m_Construct.m_Flags)
		return 0;

	// send of the packets
	m_Construct.m_Ack = m_Ack;
	m_Construct.m_Token = m_PeerToken;
	m_pNetBase->SendPacket(&m_PeerAddr, &m_Construct);

	// update send times
	m_LastSendTime = time_get();

	// clear construct so we can start building a new package
	mem_zero(&m_Construct, sizeof(m_Construct));
	return NumChunks;
}

int CNetConnection::QueueChunkEx(int Flags, int DataSize, const void *pData, int Sequence)
{
	unsigned char *pChunkData;

	// check if we have space for it, if not, flush the connection
	if(m_Construct.m_DataSize + DataSize + NET_MAX_CHUNKHEADERSIZE > (int)sizeof(m_Construct.m_aChunkData) || m_Construct.m_NumChunks == NET_MAX_PACKET_CHUNKS)
		Flush();

	// pack all the data
	CNetChunkHeader Header;
	Header.m_Flags = Flags;
	Header.m_Size = DataSize;
	Header.m_Sequence = Sequence;
	pChunkData = &m_Construct.m_aChunkData[m_Construct.m_DataSize];
	pChunkData = Header.Pack(pChunkData);
	mem_copy(pChunkData, pData, DataSize);
	pChunkData += DataSize;

	//
	m_Construct.m_NumChunks++;
	m_Construct.m_DataSize = (int)(pChunkData-m_Construct.m_aChunkData);

	return 0;
}

int CNetConnection::QueueChunk(int Flags, int DataSize, const void *pData)
{
	if(Flags&NET_CHUNKFLAG_VITAL)
		m_Sequence = (m_Sequence+1)%NET_MAX_SEQUENCE;
	return QueueChunkEx(Flags, DataSize, pData, m_Sequence);
}

void CNetConnection::SendControl(int ControlMsg, const void *pExtra, int ExtraSize)
{
	// send the control message
	m_LastSendTime = time_get();
	TOKEN PeerToken = NET_TOKEN_SOME;
	int Ack = 0;
	m_pNetBase->SendControlMsg(&m_PeerAddr, PeerToken, Ack, ControlMsg, pExtra, ExtraSize);
}

void CNetConnection::SendPacketConnless(const char *pData, int DataSize)
{
	m_pNetBase->SendPacketConnless(&m_PeerAddr, m_PeerToken, m_Token, pData, DataSize);
}

void CNetConnection::SendControlWithToken(int ControlMsg)
{
	m_LastSendTime = time_get();
	m_pNetBase->SendControlMsgWithToken(&m_PeerAddr, m_PeerToken, 0, ControlMsg, m_Token, true);
}

int CNetConnection::Connect(NETADDR *pAddr)
{
	// init connection
	Reset();
	m_LastRecvTime = time_get();
	m_PeerAddr = *pAddr;
	m_PeerToken = NET_TOKEN_NONE;
	SetToken(NET_TOKEN_SOME);
	m_State = NET_CONNSTATE_TOKEN;
	SendControlWithToken(NET_CTRLMSG_TOKEN);
	return 0;
}

void CNetConnection::Disconnect(const char *pReason)
{
	SendControl(NET_CTRLMSG_CLOSE, pReason, str_length(pReason)+1);
	Reset();
}

int CNetConnection::Feed(CNetPacketConstruct *pPacket, NETADDR *pAddr)
{
	if(pPacket->m_Token == NET_TOKEN_NONE || pPacket->m_Token != m_Token)
	{
		dbg_msg("network_in", "feed wrong token=%x expected=%x or_none=%x", pPacket->m_Token, m_Token, NET_TOKEN_NONE);
		return 0;
	}

	if(pPacket->m_Flags&NET_PACKETFLAG_CONNLESS)
	{
		dbg_msg("network_in", "feed not connless");
		return 1;
	}

	//
	if(pPacket->m_Flags&NET_PACKETFLAG_CONTROL)
	{
		int CtrlMsg = pPacket->m_aChunkData[0];
		if(CtrlMsg == NET_CTRLMSG_CLOSE)
		{
			if(net_addr_comp(&m_PeerAddr, pAddr, true) == 0)
			{
				m_State = NET_CONNSTATE_ERROR;
				m_RemoteClosed = 1;

				char Str[128] = {0};
				if(pPacket->m_DataSize > 1)
				{
					// make sure to sanitize the error string form the other party
					if(pPacket->m_DataSize < 128)
						str_copy(Str, (char *)&pPacket->m_aChunkData[1], pPacket->m_DataSize);
					else
						str_copy(Str, (char *)&pPacket->m_aChunkData[1], sizeof(Str));
					str_sanitize_strong(Str);
				}

				dbg_msg("conn", "closed reason='%s'", Str);
			}
			return 0;
		}
		else
		{
			if(CtrlMsg == NET_CTRLMSG_TOKEN)
			{
				m_PeerToken = pPacket->m_ResponseToken;

				if(State() == NET_CONNSTATE_TOKEN)
				{
					m_State = NET_CONNSTATE_CONNECT;
					SendControlWithToken(NET_CTRLMSG_CONNECT);
					dbg_msg("connection", "got token, replying, token=%x mytoken=%x", m_PeerToken, m_Token);
				}
				else
					dbg_msg("connection", "got token, token=%x", m_PeerToken);
			}
			else if(CtrlMsg == NET_CTRLMSG_CONNECT)
			{
				// send response and init connection
				TOKEN Token = m_Token;
				Reset();
				m_State = NET_CONNSTATE_PENDING;
				m_PeerAddr = *pAddr;
				m_PeerToken = pPacket->m_ResponseToken;
				m_Token = Token;
				SendControl(NET_CTRLMSG_ACCEPT, 0, 0);
				dbg_msg("connection", "got connection, sending accept");
			}
			// connection made
			else if(CtrlMsg == NET_CTRLMSG_ACCEPT)
			{
				m_State = NET_CONNSTATE_ONLINE;
				dbg_msg("connection", "got accept. connection online");
			}
		}
	}
	else
	{
		if(State() == NET_CONNSTATE_PENDING)
		{
			m_State = NET_CONNSTATE_ONLINE;
			dbg_msg("connection", "connecting online");
		}
	}
	return 1;
}

int CNetConnection::IsSeqInBackroom(int Seq, int Ack)
{
	int Bottom = (Ack - NET_MAX_SEQUENCE / 2);
	if(Bottom < 0)
	{
		if(Seq <= Ack)
			return 1;
		if(Seq >= (Bottom + NET_MAX_SEQUENCE))
			return 1;
	}
	else
	{
		if(Seq <= Ack && Seq >= Bottom)
			return 1;
	}

	return 0;
}
