/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>

#include <engine/engine.h>

#include "config.h"
#include "console.h"
#include "network.h"
#include "huffman.h"


static void ConchainDbgLognetwork(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	((CNetBase *)pUserData)->UpdateLogHandles();
}

void CNetRecvUnpacker::Clear()
{
	m_Valid = false;
}

void CNetRecvUnpacker::Start(const NETADDR *pAddr, CNetConnection *pConnection, int ClientID)
{
	m_Addr = *pAddr;
	m_pConnection = pConnection;
	m_ClientID = ClientID;
	m_CurrentChunk = 0;
	m_Valid = true;
}

// TODO: rename this function
int CNetRecvUnpacker::FetchChunk(CNetChunk *pChunk)
{
	// Don't bother with connections that already went offline
	if(m_pConnection && m_pConnection->State() != NET_CONNSTATE_ONLINE)
	{
		Clear();
		return 0;
	}

	CNetChunkHeader Header;
	unsigned char *pEnd = m_Data.m_aChunkData + m_Data.m_DataSize;

	while(1)
	{
		unsigned char *pData = m_Data.m_aChunkData;

		// check for old data to unpack
		if(!m_Valid || m_CurrentChunk >= m_Data.m_NumChunks)
		{
			Clear();
			return 0;
		}

		// TODO: add checking here so we don't read too far
		for(int i = 0; i < m_CurrentChunk; i++)
		{
			pData = Header.Unpack(pData);
			pData += Header.m_Size;
		}

		// unpack the header
		pData = Header.Unpack(pData);
		m_CurrentChunk++;

		if(pData+Header.m_Size > pEnd)
		{
			Clear();
			return 0;
		}

		// handle sequence stuff
		if(m_pConnection && (Header.m_Flags&NET_CHUNKFLAG_VITAL))
		{
			if(Header.m_Sequence == (m_pConnection->m_Ack+1)%NET_MAX_SEQUENCE)
			{
				// in sequence
				m_pConnection->m_Ack = (m_pConnection->m_Ack+1)%NET_MAX_SEQUENCE;
			}
			else
			{
				// old packet that we already got
				if(m_pConnection->IsSeqInBackroom(Header.m_Sequence, m_pConnection->m_Ack))
					continue;

				// out of sequence, request resend
				if(m_pConnection->Config()->m_Debug)
					dbg_msg("conn", "asking for resend %d %d", Header.m_Sequence, (m_pConnection->m_Ack+1)%NET_MAX_SEQUENCE);
				m_pConnection->SignalResend();
				continue; // take the next chunk in the packet
			}
		}

		// fill in the info
		pChunk->m_ClientID = m_ClientID;
		pChunk->m_Address = m_Addr;
		pChunk->m_Flags = (Header.m_Flags&NET_CHUNKFLAG_VITAL) ? NETSENDFLAG_VITAL : 0;
		pChunk->m_DataSize = Header.m_Size;
		pChunk->m_pData = pData;
		return 1;
	}
}
CNetBase::CNetInitializer CNetBase::m_NetInitializer;

CNetBase::CNetBase()
{
	net_invalidate_socket(&m_Socket);
	m_pConfig = 0;
	m_pEngine = 0;
	m_DataLogSent = 0;
	m_DataLogRecv = 0;
}

CNetBase::~CNetBase()
{
	if(m_Socket.type != NETTYPE_INVALID)
		Shutdown();
}

void CNetBase::Init(NETSOCKET Socket, CConfig *pConfig, IConsole *pConsole, IEngine *pEngine)
{
	m_Socket = Socket;
	m_pConfig = pConfig;
	m_pEngine = pEngine;
	m_Huffman.Init();
	mem_zero(m_aRequestTokenBuf, sizeof(m_aRequestTokenBuf));
	if(pEngine)
		pConsole->Chain("dbg_lognetwork", ConchainDbgLognetwork, this);
}

void CNetBase::Shutdown()
{
	net_udp_close(m_Socket);
	net_invalidate_socket(&m_Socket);
}

void CNetBase::Wait(int Time)
{
	net_socket_read_wait(m_Socket, Time);
}

// packs the data tight and sends it
void CNetBase::SendPacketConnless(const NETADDR *pAddr, TOKEN Token, TOKEN ResponseToken, const void *pData, int DataSize)
{
	unsigned char aBuffer[NET_MAX_PACKETSIZE];

	dbg_assert(DataSize <= NET_MAX_PAYLOAD, "packet data size too high");
	dbg_assert((Token&~NET_TOKEN_MASK) == 0, "token out of range");
	dbg_assert((ResponseToken&~NET_TOKEN_MASK) == 0, "resp token out of range");

	int i = 0;
	aBuffer[i++] = ((NET_PACKETFLAG_CONNLESS<<2)&0xfc) | (NET_PACKETVERSION&0x03); // connless flag and version
	aBuffer[i++] = (Token>>24)&0xff; // token
	aBuffer[i++] = (Token>>16)&0xff;
	aBuffer[i++] = (Token>>8)&0xff;
	aBuffer[i++] = (Token)&0xff;
	aBuffer[i++] = (ResponseToken>>24)&0xff; // response token
	aBuffer[i++] = (ResponseToken>>16)&0xff;
	aBuffer[i++] = (ResponseToken>>8)&0xff;
	aBuffer[i++] = (ResponseToken)&0xff;

	dbg_assert(i == NET_PACKETHEADERSIZE_CONNLESS, "inconsistency");

	mem_copy(&aBuffer[i], pData, DataSize);
	net_udp_send(m_Socket, pAddr, aBuffer, i+DataSize);
}

void CNetBase::SendPacket(const NETADDR *pAddr, CNetPacketConstruct *pPacket)
{
	unsigned char aBuffer[NET_MAX_PACKETSIZE];
	int CompressedSize = -1;
	int FinalSize = -1;

	// log the data
	if(m_DataLogSent)
	{
		int Type = 1;
		io_write(m_DataLogSent, &Type, sizeof(Type));
		io_write(m_DataLogSent, &pPacket->m_DataSize, sizeof(pPacket->m_DataSize));
		io_write(m_DataLogSent, &pPacket->m_aChunkData, pPacket->m_DataSize);
		io_flush(m_DataLogSent);
	}

	dbg_assert((pPacket->m_Token&~NET_TOKEN_MASK) == 0, "token out of range");

	// compress if not ctrl msg
	if(!(pPacket->m_Flags&NET_PACKETFLAG_CONTROL))
		CompressedSize = m_Huffman.Compress(pPacket->m_aChunkData, pPacket->m_DataSize, &aBuffer[NET_PACKETHEADERSIZE], NET_MAX_PAYLOAD);

	// check if the compression was enabled, successful and good enough
	if(CompressedSize > 0 && CompressedSize < pPacket->m_DataSize)
	{
		FinalSize = CompressedSize;
		pPacket->m_Flags |= NET_PACKETFLAG_COMPRESSION;
	}
	else
	{
		// use uncompressed data
		FinalSize = pPacket->m_DataSize;
		mem_copy(&aBuffer[NET_PACKETHEADERSIZE], pPacket->m_aChunkData, pPacket->m_DataSize);
		pPacket->m_Flags &= ~NET_PACKETFLAG_COMPRESSION;
	}

	// set header and send the packet if all things are good
	if(FinalSize >= 0)
	{
		FinalSize += NET_PACKETHEADERSIZE;

		int i = 0;
		aBuffer[i++] = ((pPacket->m_Flags<<2)&0xfc) | ((pPacket->m_Ack>>8)&0x03); // flags and ack
		aBuffer[i++] = (pPacket->m_Ack)&0xff; // ack
		aBuffer[i++] = (pPacket->m_NumChunks)&0xff; // num chunks
		aBuffer[i++] = (pPacket->m_Token>>24)&0xff; // token
		aBuffer[i++] = (pPacket->m_Token>>16)&0xff;
		aBuffer[i++] = (pPacket->m_Token>>8)&0xff;
		aBuffer[i++] = (pPacket->m_Token)&0xff;

		dbg_assert(i == NET_PACKETHEADERSIZE, "inconsistency");

		net_udp_send(m_Socket, pAddr, aBuffer, FinalSize);

		// log raw socket data
		if(m_DataLogSent)
		{
			int Type = 0;
			io_write(m_DataLogSent, &Type, sizeof(Type));
			io_write(m_DataLogSent, &FinalSize, sizeof(FinalSize));
			io_write(m_DataLogSent, aBuffer, FinalSize);
			io_flush(m_DataLogSent);
		}
	}

	if(m_pConfig->m_Debug > 2)
	{
		PrintPacket(pPacket, aBuffer, FinalSize, pAddr, NETWORK_OUT);
	}
}

// TODO: rename this function
int CNetBase::UnpackPacket(NETADDR *pAddr, unsigned char *pBuffer, CNetPacketConstruct *pPacket)
{
	int Size = net_udp_recv(m_Socket, pAddr, pBuffer, NET_MAX_PACKETSIZE);
	// no more packets for now
	if(Size <= 0)
		return 1;

	// log the data
	if(m_DataLogRecv)
	{
		int Type = 0;
		io_write(m_DataLogRecv, &Type, sizeof(Type));
		io_write(m_DataLogRecv, &Size, sizeof(Size));
		io_write(m_DataLogRecv, pBuffer, Size);
		io_flush(m_DataLogRecv);
	}

	// check the size
	if(Size < NET_PACKETHEADERSIZE || Size > NET_MAX_PACKETSIZE)
	{
		if(m_pConfig->m_Debug)
			dbg_msg("network", "packet too small, size=%d", Size);
		return -1;
	}

	// read the packet

	pPacket->m_Flags = (pBuffer[0]&0xfc)>>2;

		// FFFFFFxx
	if(pPacket->m_Flags&NET_PACKETFLAG_CONNLESS)
	{
		if(Size < NET_PACKETHEADERSIZE_CONNLESS)
		{
			if(m_pConfig->m_Debug)
				dbg_msg("net", "connless packet too small, size=%d", Size);
			return -1;
		}

		pPacket->m_Flags = NET_PACKETFLAG_CONNLESS;
		pPacket->m_Ack = 0;
		pPacket->m_NumChunks = 0;
		int Version = pBuffer[0]&0x3;
			// xxxxxxVV

		if(Version != NET_PACKETVERSION)
		{
			if(m_pConfig->m_Debug > 2)
			{
				if(ShowAddr(pAddr))
					dbg_msg("network_in", "got invalid version %d expected %d", Version, NET_PACKETVERSION);
			}
			return -1;
		}

		pPacket->m_DataSize = Size - NET_PACKETHEADERSIZE_CONNLESS;
		pPacket->m_Token = (pBuffer[1] << 24) | (pBuffer[2] << 16) | (pBuffer[3] << 8) | pBuffer[4];
			// TTTTTTTT TTTTTTTT TTTTTTTT TTTTTTTT
		pPacket->m_ResponseToken = (pBuffer[5]<<24) | (pBuffer[6]<<16) | (pBuffer[7]<<8) | pBuffer[8];
			// RRRRRRRR RRRRRRRR RRRRRRRR RRRRRRRR
		mem_copy(pPacket->m_aChunkData, &pBuffer[NET_PACKETHEADERSIZE_CONNLESS], pPacket->m_DataSize);
	}
	else
	{
		if(Size - NET_PACKETHEADERSIZE > NET_MAX_PAYLOAD)
		{
			if(m_pConfig->m_Debug)
				dbg_msg("network", "packet payload too big, size=%d", Size);
			return -1;
		}

		pPacket->m_Ack = ((pBuffer[0]&0x3)<<8) | pBuffer[1];
			// xxxxxxAA AAAAAAAA
		pPacket->m_NumChunks = pBuffer[2];
			// NNNNNNNN

		pPacket->m_DataSize = Size - NET_PACKETHEADERSIZE;
		pPacket->m_Token = (pBuffer[3] << 24) | (pBuffer[4] << 16) | (pBuffer[5] << 8) | pBuffer[6];
			// TTTTTTTT TTTTTTTT TTTTTTTT TTTTTTTT
		pPacket->m_ResponseToken = NET_TOKEN_NONE;

		if(pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION)
			pPacket->m_DataSize = m_Huffman.Decompress(&pBuffer[NET_PACKETHEADERSIZE], pPacket->m_DataSize, pPacket->m_aChunkData, sizeof(pPacket->m_aChunkData));
		else
			mem_copy(pPacket->m_aChunkData, &pBuffer[NET_PACKETHEADERSIZE], pPacket->m_DataSize);
	}

	// check for errors
	if(pPacket->m_DataSize < 0)
	{
		if(m_pConfig->m_Debug)
			dbg_msg("network", "error during packet decoding");
		return -1;
	}

	// set the response token (a bit hacky because this function shouldn't know about control packets)
	if(pPacket->m_Flags&NET_PACKETFLAG_CONTROL)
	{
		if(pPacket->m_DataSize >= 5) // control byte + token
		{
			if(pPacket->m_aChunkData[0] == NET_CTRLMSG_CONNECT
				|| pPacket->m_aChunkData[0] == NET_CTRLMSG_TOKEN)
			{
				pPacket->m_ResponseToken = (pPacket->m_aChunkData[1]<<24) | (pPacket->m_aChunkData[2]<<16)
					| (pPacket->m_aChunkData[3]<<8) | pPacket->m_aChunkData[4];
			}
		}
	}

	if(m_pConfig->m_Debug > 1)
	{
		PrintPacket(pPacket, pBuffer, Size, pAddr, NETWORK_IN);
	}

	// log the data
	if(m_DataLogRecv)
	{
		int Type = 1;
		io_write(m_DataLogRecv, &Type, sizeof(Type));
		io_write(m_DataLogRecv, &pPacket->m_DataSize, sizeof(pPacket->m_DataSize));
		io_write(m_DataLogRecv, pPacket->m_aChunkData, pPacket->m_DataSize);
		io_flush(m_DataLogRecv);
	}

	// return success
	return 0;
}


void CNetBase::SendControlMsg(const NETADDR *pAddr, TOKEN Token, int Ack, int ControlMsg, const void *pExtra, int ExtraSize)
{
	CNetPacketConstruct Construct;
	Construct.m_Token = Token;
	Construct.m_Flags = NET_PACKETFLAG_CONTROL;
	Construct.m_Ack = Ack;
	Construct.m_NumChunks = 0;
	Construct.m_DataSize = 1+ExtraSize;
	Construct.m_aChunkData[0] = ControlMsg;
	if(ExtraSize > 0)
	{
		mem_copy(&Construct.m_aChunkData[1], pExtra, ExtraSize);
		/*
			When SendControlMsgWithToken() is called
			the ExtraSize is 4 and the pExtra is the token

			so the first packet the server sends is
			
			packet header - 1 byte indicating its control message - 4 byte token

			and the client answers in the same format
		*/
		// if(Config()->m_Debug > 2 && ExtraSize < 512)
		// {
		// 	char aHex[512];
		// 	str_hex(aHex, sizeof(aHex), pExtra, ExtraSize);
		// 	dbg_msg("network_out", "SendControlMsg with extrasize=%d data=%s", ExtraSize, aHex);
		// }
	}

	// send the control message
	SendPacket(pAddr, &Construct);
}


void CNetBase::SendControlMsgWithToken(const NETADDR *pAddr, TOKEN Token, int Ack, int ControlMsg, TOKEN MyToken, bool Extended)
{
	dbg_assert((Token&~NET_TOKEN_MASK) == 0, "token out of range");
	dbg_assert((MyToken&~NET_TOKEN_MASK) == 0, "resp token out of range");

	m_aRequestTokenBuf[0] = (MyToken>>24)&0xff;
	m_aRequestTokenBuf[1] = (MyToken>>16)&0xff;
	m_aRequestTokenBuf[2] = (MyToken>>8)&0xff;
	m_aRequestTokenBuf[3] = (MyToken)&0xff;
	SendControlMsg(pAddr, Token, 0, ControlMsg, m_aRequestTokenBuf, Extended ? sizeof(m_aRequestTokenBuf) : 4);
}

unsigned char *CNetChunkHeader::Pack(unsigned char *pData)
{
	pData[0] = ((m_Flags&0x03)<<6) | ((m_Size>>6)&0x3F);
	pData[1] = (m_Size&0x3F);
	if(m_Flags&NET_CHUNKFLAG_VITAL)
	{
		pData[1] |= (m_Sequence>>2)&0xC0;
		pData[2] = m_Sequence&0xFF;
		return pData + 3;
	}
	return pData + 2;
}

unsigned char *CNetChunkHeader::Unpack(unsigned char *pData)
{
	m_Flags = (pData[0]>>6)&0x03;
	m_Size = ((pData[0]&0x3F)<<6) | (pData[1]&0x3F);
	// const char *pFlags = "none";
	// if(m_Flags&NET_CHUNKFLAG_VITAL)
	// 	pFlags = "NET_CHUNKFLAG_VITAL";
	// else if(m_Flags&NET_CHUNKFLAG_RESEND)
	// 	pFlags = "NET_CHUNKFLAG_RESEND";
	// print_hex_row_highlighted("network_in", "  ChunkHeader[0-1]: ", pData, 2, 0, 0, "flags = %d (%s)", (pData[0]>>6)&0x03, pFlags);
	// print_hex_row_highlighted("network_in", "  ChunkHeader[0-1]: ", pData, 2, 0, 1, "size = %d ((header[0]&0x3F)<<6) | (header[1]&0x3F)", ((pData[0]&0x3F)<<6) | (pData[1]&0x3F));
	m_Sequence = -1;
	if(m_Flags&NET_CHUNKFLAG_VITAL)
	{
		m_Sequence = ((pData[1]&0xC0)<<2) | pData[2];
		return pData + 3;
	}
	return pData + 2;
}

void CNetBase::UpdateLogHandles()
{
	if(Engine())
		Engine()->QueryNetLogHandles(&m_DataLogSent, &m_DataLogRecv);
}

bool CNetBase::ShowAddr(const NETADDR *pAddr)
{
	char aAddrStr[NETADDR_MAXSTRSIZE];
	net_addr_str(pAddr, aAddrStr, sizeof(aAddrStr), true);
	return str_startswith(aAddrStr, "[0:0:0:0:0:0:0:1]:") || str_startswith(aAddrStr, "127.0.0.1:");
}

void CNetBase::PrintPacket(CNetPacketConstruct *pPacket, unsigned char *pPacketData, int PacketSize, const NETADDR *pAddr, ENetDirection Direction)
{
	if(!ShowAddr(pAddr))
		return;

	char aAddrStr[NETADDR_MAXSTRSIZE];
	net_addr_str(pAddr, aAddrStr, sizeof(aAddrStr), true);
	char aBuf[512];
	aBuf[0] = '\0';
	if(pPacket->m_Flags&NET_PACKETFLAG_CONTROL)
		str_append(aBuf, "CONTROL", sizeof(aBuf));
	if(pPacket->m_Flags&NET_PACKETFLAG_RESEND)
		str_append(aBuf, aBuf[0] ? "|RESEND" : "RESEND", sizeof(aBuf));
	if(pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION)
		str_append(aBuf, aBuf[0] ? "|COMPRESSION" : "COMPRESSION", sizeof(aBuf));
	if(pPacket->m_Flags&NET_PACKETFLAG_CONNLESS)
		str_append(aBuf, aBuf[0] ? "|CONNLESS" : "CONNLESS", sizeof(aBuf));
	char aFlags[512];
	aFlags[0] = '\0';
	if(aBuf[0])
		str_format(aFlags, sizeof(aFlags), " (%s)", aBuf);
	int PacketHeaderSize = pPacket->m_Flags&NET_PACKETFLAG_CONNLESS ? NET_PACKETHEADERSIZE_CONNLESS : NET_PACKETHEADERSIZE;
	int RowLen = 12;
	int NullBytes = maximum(pPacket->m_DataSize - RowLen, 0);
	for(int i = RowLen; i < pPacket->m_DataSize; i++)
	{
		if(pPacket->m_aChunkData[i] != 0x0)
		{
			NullBytes = 0;
			break;
		}
	}
	int PrintPacketLen = minimum(NullBytes ? RowLen : PacketSize, PacketSize);
	int PrintDataLen = minimum(NullBytes ? RowLen : pPacket->m_DataSize, pPacket->m_DataSize);
	char aHexData[1024];
	str_hex(aHexData, sizeof(aHexData), pPacket->m_aChunkData, PrintDataLen);
	char aRawData[1024] = {0};
	for(int i = 0; i < PrintDataLen; i++)
		aRawData[i] = (pPacket->m_aChunkData[i] < 32 || pPacket->m_aChunkData[i] > 126) ? '.' : pPacket->m_aChunkData[i];
	dbg_msg(Direction == NETWORK_IN ? "network_in" : "network_out", "%s packetsize=%d datasize=%d flags=%d%s", aAddrStr, PacketSize, pPacket->m_DataSize, pPacket->m_Flags, aFlags);
	char aInfo[512];
	aInfo[0] = '\0';
	if(NullBytes)
		str_format(aInfo, sizeof(aInfo), "  [CUT OFF %d NULL BYTES AFTER THE FIRST ROW]", NullBytes);
	if(pPacket->m_DataSize < 20 || NullBytes)
	{
		if(pPacket->m_Flags&NET_PACKETFLAG_CONTROL)
		{
			int CtrlMsg = pPacket->m_aChunkData[0];
			const char *pMsg = "unkown";
			if(CtrlMsg == NET_CTRLMSG_KEEPALIVE) { pMsg = "NET_CTRLMSG_KEEPALIVE"; }
			else if(CtrlMsg == NET_CTRLMSG_CONNECT) { pMsg = "NET_CTRLMSG_CONNECT"; }
			else if(CtrlMsg == NET_CTRLMSG_ACCEPT) { pMsg = "NET_CTRLMSG_ACCEPT"; }
			else if(CtrlMsg == NET_CTRLMSG_CLOSE) { pMsg = "NET_CTRLMSG_CLOSE"; }
			else if(CtrlMsg == NET_CTRLMSG_TOKEN) { pMsg = "NET_CTRLMSG_TOKEN"; }
			char aPacketHeader[1024];
			char aCtrlMsg[1024];
			str_format(aPacketHeader, sizeof(aPacketHeader), "PHeader: size=%d flags=%d", PacketHeaderSize, pPacket->m_Flags);
			str_format(aCtrlMsg, sizeof(aCtrlMsg), "CtrlMsg = %d (%s)", CtrlMsg, pMsg);
			if(CtrlMsg == NET_CTRLMSG_TOKEN || CtrlMsg == NET_CTRLMSG_CONNECT)
			{
				print_hex_row_highlight_three(
					Direction == NETWORK_IN ? "network_in" : "network_out",
					pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION ? "  full_packet_compressed: " : "  full_packet: ",
					pPacketData, PrintPacketLen,
					0, PacketHeaderSize - 1, /* print_hex_row_highlighted expects indecies so from 0 - 6 = 7 chunks */
					aPacketHeader,
					PacketHeaderSize, PacketHeaderSize,
					aCtrlMsg,
					PacketHeaderSize + 1,
					PrintPacketLen - 1,
					"token",
					aInfo);
			}
			else
			{
				print_hex_row_highlight_two(
					Direction == NETWORK_IN ? "network_in" : "network_out",
					pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION ? "  full_packet_compressed: " : "  full_packet: ",
					pPacketData, PrintPacketLen,
					0, PacketHeaderSize - 1, /* print_hex_row_highlighted expects indecies so from 0 - 6 = 7 chunks */
					aPacketHeader,
					PacketHeaderSize, PacketHeaderSize,
					aCtrlMsg,
					aInfo);
			}
		}
		else
		{
			print_hex_row_highlighted(
				Direction == NETWORK_IN ? "network_in" : "network_out",
				pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION ? "  full_packet_compressed: " : "  full_packet: ",
				pPacketData, PrintPacketLen,
				0, PacketHeaderSize - 1, /* print_hex_row_highlighted expects indecies so from 0 - 6 = 7 chunks */
				"PacketHeader: size=%d flags = %d%s", PacketHeaderSize, pPacket->m_Flags, aFlags);
		}
		dbg_msg(Direction == NETWORK_IN ? "network_in" : "network_out",
			"  %sdata_raw: %s",
			pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION ? "decompressed_" : "",
			aRawData);
		dbg_msg(Direction == NETWORK_IN ? "network_in" : "network_out",
			"  %sdata_hex: %s",
			pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION ? "decompressed_" : "",
			aHexData);
		// TODO: fix the condition when we parse the chunk header
		//		 the compression flags seems more like correlation than actual cause
		// 		 for the chunk header to exist
		//		 to fix this make sure this branch is only called when
		// 		 CNetChunkHeader::Unpack() is being called
		if(pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION)
		{
			// from CNetChunkHeader::Unpack
			// TODO: these values are probably somewhere in pPacket
			int ChunkHeaderFlags = (pPacket->m_aChunkData[0]>>6)&0x03;
			int ChunkHeaderSize = ((pPacket->m_aChunkData[0]&0x3F)<<6) | (pPacket->m_aChunkData[1]&0x3F);
			const char *pFlags = "none";
			if(ChunkHeaderFlags&NET_CHUNKFLAG_VITAL)
				pFlags = "NET_CHUNKFLAG_VITAL";
			else if(ChunkHeaderFlags&NET_CHUNKFLAG_RESEND)
				pFlags = "NET_CHUNKFLAG_RESEND";
			print_hex_row_highlighted(
				Direction == NETWORK_IN ? "network_in" : "network_out",
				pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION ? "  decompressed_data_hex: " : "  data_hex: ",
				pPacket->m_aChunkData, PrintDataLen,
				0, 1, // TODO: chunk header is 3 long if flag vital is set
				"ChunkHeader: size = %d flags = %d (%s)", ChunkHeaderSize, ChunkHeaderFlags, pFlags);
		}
		else if(!(pPacket->m_Flags&NET_PACKETFLAG_CONTROL))
		{
			// seems like when there is no compression
			// there is no chunk header
			print_hex_row_highlighted(
				Direction == NETWORK_IN ? "network_in" : "network_out",
				"  data_hex: ",
				pPacket->m_aChunkData, PrintDataLen,
				0, 0,
				"TODO: no idea what this is");
		}
	}
	else
	{
		dbg_msg(Direction == NETWORK_IN ? "network_in" : "network_out", "  decompressed_data:");
		print_hex(Direction == NETWORK_IN ? "network_in" : "network_out", "    ", pPacket->m_aChunkData, pPacket->m_DataSize, RowLen);
	}
}

void print_state(const char *type, const char *note, int State)
{
	const char *pState = "unknown";
	if (State == NET_CONNSTATE_OFFLINE)
		pState = "NET_CONNSTATE_OFFLINE";
	if (State == NET_CONNSTATE_TOKEN)
		pState = "NET_CONNSTATE_TOKEN";
	if (State == NET_CONNSTATE_CONNECT)
		pState = "NET_CONNSTATE_CONNECT";
	if (State == NET_CONNSTATE_PENDING)
		pState = "NET_CONNSTATE_PENDING";
	if (State == NET_CONNSTATE_ONLINE)
		pState = "NET_CONNSTATE_ONLINE";
	if (State == NET_CONNSTATE_ERROR)
		pState = "NET_CONNSTATE_ERROR";
	dbg_msg(type, "%s state=%d (%s)", note, State, pState);
}
