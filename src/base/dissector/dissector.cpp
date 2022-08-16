// ChillerDragon 2022 - dissector


#include "dissector.h"

#include <engine/shared/packer.h>
#include <engine/shared/network.h>
#include <base/math.h>
#include <engine/shared/config.h>

void print_packet(CNetPacketConstruct *pPacket, unsigned char *pPacketData, int PacketSize, const NETADDR *pAddr, ENetDirection Direction, const CConfig *pConfig)
{
	if(!show_addr(pAddr))
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
	str_raw(aRawData, sizeof(aRawData), pPacket->m_aChunkData, PrintDataLen);
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
			// Hide keepalive if debug is lower than 4
			// It gets spammy and makes it harder to read the more interesting messages
			if(CtrlMsg == NET_CTRLMSG_KEEPALIVE) { pMsg = "NET_CTRLMSG_KEEPALIVE"; if (pConfig->m_Debug < 4) return; }
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
			if(pPacket->m_DataSize == 4)
			{
				char aMsgInfo[512];
				str_format(aMsgInfo, sizeof(aMsgInfo), "MsgID = TODO");
				print_hex_row_highlight_two(
					Direction == NETWORK_IN ? "network_in" : "network_out",
					"  data_hex: ",
					pPacket->m_aChunkData, PrintDataLen,
					0, 2,
					"CHeader",
					3, 3,
					aMsgInfo, "");
			}
			else
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
		print_hex_row_highlight_two(
					Direction == NETWORK_IN ? "network_in" : "network_out",
					pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION ? "  full_packet_compressed: " : "  full_packet: ",
					pPacketData, minimum(PrintPacketLen, 14),
					0, PacketHeaderSize - 1, /* print_hex_row_highlighted expects indecies so from 0 - 6 = 7 chunks */
					"header",
					PacketHeaderSize, PacketHeaderSize,
					pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION ? "compressed data from here on" : "who dis?",
					"[ONLY FIRST ROW DISPLAYED]");
		if(pPacket->m_Flags&NET_PACKETFLAG_COMPRESSION)
		{
			// unpack msg and sys code from CClient::ProcessServerPacket()
			CUnpacker Unpacker;
			// first two bytes are Flags and Size ignore those and skip to byte 3
			Unpacker.Reset(pPacket->m_aChunkData + 2, pPacket->m_DataSize - 2);

			// unpack msgid and system flag
			int Msg = Unpacker.GetInt();
			int Sys = Msg&1;
			Msg >>= 1;

			char aMsg[512];
			char aMsgName[512];
			netmsg_to_s(Msg, aMsgName, sizeof(aMsgName));
			str_format(aMsg, sizeof(aMsg), "Msg=%d (%s) Sys=%d", Msg, aMsgName, Sys);

			dbg_msg(Direction == NETWORK_IN ? "network_in" : "network_out", "  decompressed_data:");
			print_hex_row_highlight_two(
					Direction == NETWORK_IN ? "network_in" : "network_out",
					"    ",
					pPacket->m_aChunkData, RowLen,
					0, 1,
					"Flags & Size",
					2, 2,
					aMsg,
					"");
			print_hex(Direction == NETWORK_IN ? "network_in" : "network_out", "    ", pPacket->m_aChunkData + RowLen, pPacket->m_DataSize - RowLen, RowLen);
		}
		else
		{
			dbg_msg(Direction == NETWORK_IN ? "network_in" : "network_out", "  data:");
			print_hex(Direction == NETWORK_IN ? "network_in" : "network_out", "    ", pPacket->m_aChunkData, pPacket->m_DataSize, RowLen);
		}
	}
}

bool show_addr(const NETADDR *pAddr)
{
	char aAddrStr[NETADDR_MAXSTRSIZE];
	net_addr_str(pAddr, aAddrStr, sizeof(aAddrStr), true);
	return str_startswith(aAddrStr, "[0:0:0:0:0:0:0:1]:") || str_startswith(aAddrStr, "127.0.0.1:");
}

void netmsg_to_s(int Msg, char *pBuf, int Size)
{
    if(Msg == NETMSG_NULL) { str_copy(pBuf, "NETMSG_NULL", Size); }
    else if(Msg == NETMSG_INFO) { str_copy(pBuf, "NETMSG_INFO", Size); }
    else if(Msg == NETMSG_MAP_CHANGE) { str_copy(pBuf, "NETMSG_MAP_CHANGE", Size); }
    else if(Msg == NETMSG_MAP_DATA) { str_copy(pBuf, "NETMSG_MAP_DATA", Size); }
    else if(Msg == NETMSG_SERVERINFO) { str_copy(pBuf, "NETMSG_SERVERINFO", Size); }
    else if(Msg == NETMSG_CON_READY) { str_copy(pBuf, "NETMSG_CON_READY", Size); }
    else if(Msg == NETMSG_SNAP) { str_copy(pBuf, "NETMSG_SNAP", Size); }
    else if(Msg == NETMSG_SNAPEMPTY) { str_copy(pBuf, "NETMSG_SNAPEMPTY", Size); }
    else if(Msg == NETMSG_SNAPSINGLE) { str_copy(pBuf, "NETMSG_SNAPSINGLE", Size); }
    else if(Msg == NETMSG_SNAPSMALL) { str_copy(pBuf, "NETMSG_SNAPSMALL", Size); }
    else if(Msg == NETMSG_INPUTTIMING) { str_copy(pBuf, "NETMSG_INPUTTIMING", Size); }
    else if(Msg == NETMSG_RCON_AUTH_ON) { str_copy(pBuf, "NETMSG_RCON_AUTH_ON", Size); }
    else if(Msg == NETMSG_RCON_AUTH_OFF) { str_copy(pBuf, "NETMSG_RCON_AUTH_OFF", Size); }
    else if(Msg == NETMSG_RCON_LINE) { str_copy(pBuf, "NETMSG_RCON_LINE", Size); }
    else if(Msg == NETMSG_RCON_CMD_ADD) { str_copy(pBuf, "NETMSG_RCON_CMD_ADD", Size); }
    else if(Msg == NETMSG_RCON_CMD_REM) { str_copy(pBuf, "NETMSG_RCON_CMD_REM", Size); }
    else if(Msg == NETMSG_AUTH_CHALLANGE) { str_copy(pBuf, "NETMSG_AUTH_CHALLANGE", Size); }
    else if(Msg == NETMSG_AUTH_RESULT) { str_copy(pBuf, "NETMSG_AUTH_RESULT", Size); }
    else if(Msg == NETMSG_READY) { str_copy(pBuf, "NETMSG_READY", Size); }
    else if(Msg == NETMSG_ENTERGAME) { str_copy(pBuf, "NETMSG_ENTERGAME", Size); }
    else if(Msg == NETMSG_INPUT) { str_copy(pBuf, "NETMSG_INPUT", Size); }
    else if(Msg == NETMSG_RCON_CMD) { str_copy(pBuf, "NETMSG_RCON_CMD", Size); }
    else if(Msg == NETMSG_RCON_AUTH) { str_copy(pBuf, "NETMSG_RCON_AUTH", Size); }
    else if(Msg == NETMSG_REQUEST_MAP_DATA) { str_copy(pBuf, "NETMSG_REQUEST_MAP_DATA", Size); }
    else if(Msg == NETMSG_AUTH_START) { str_copy(pBuf, "NETMSG_AUTH_START", Size); }
    else if(Msg == NETMSG_AUTH_RESPONSE) { str_copy(pBuf, "NETMSG_AUTH_RESPONSE", Size); }
    else if(Msg == NETMSG_PING) { str_copy(pBuf, "NETMSG_PING", Size); }
    else if(Msg == NETMSG_PING_REPLY) { str_copy(pBuf, "NETMSG_PING_REPLY", Size); }
    else if(Msg == NETMSG_ERROR) { str_copy(pBuf, "NETMSG_ERROR", Size); }
    else if(Msg == NETMSG_MAPLIST_ENTRY_ADD) { str_copy(pBuf, "NETMSG_MAPLIST_ENTRY_ADD", Size); }
    else if(Msg == NETMSG_MAPLIST_ENTRY_REM) { str_copy(pBuf, "NETMSG_MAPLIST_ENTRY_REM", Size); }
    else { str_copy(pBuf, "unknown", Size); }
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
