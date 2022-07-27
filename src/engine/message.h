/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_MESSAGE_H
#define ENGINE_MESSAGE_H

#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>

class CMsgPacker : public CPacker
{
public:
	CMsgPacker(int Type, bool System=false)
	{
		const char *pMsg = "unkown";
		if(Type == NETMSG_NULL) { pMsg = "NULL"; }
		else if(Type == NETMSG_INFO) { pMsg = "INFO"; }
		else if(Type == NETMSG_MAP_CHANGE) { pMsg = "MAP_CHANGE"; }
		else if(Type == NETMSG_MAP_DATA) { pMsg = "MAP_DATA"; }
		else if(Type == NETMSG_SERVERINFO) { pMsg = "SERVERINFO"; }
		else if(Type == NETMSG_CON_READY) { pMsg = "CON_READY"; }
		else if(Type == NETMSG_SNAP) { pMsg = "SNAP"; }
		else if(Type == NETMSG_SNAPEMPTY) { pMsg = "SNAPEMPTY"; }
		else if(Type == NETMSG_SNAPSINGLE) { pMsg = "SNAPSINGLE"; }
		else if(Type == NETMSG_SNAPSMALL) { pMsg = "SNAPSMALL"; }
		else if(Type == NETMSG_INPUTTIMING) { pMsg = "INPUTTIMING"; }
		else if(Type == NETMSG_RCON_AUTH_ON) { pMsg = "RCON_AUTH_ON"; }
		else if(Type == NETMSG_RCON_AUTH_OFF) { pMsg = "RCON_AUTH_OFF"; }
		else if(Type == NETMSG_RCON_LINE) { pMsg = "RCON_LINE"; }
		else if(Type == NETMSG_RCON_CMD_ADD) { pMsg = "RCON_CMD_ADD"; }
		else if(Type == NETMSG_RCON_CMD_REM) { pMsg = "RCON_CMD_REM"; }
		else if(Type == NETMSG_AUTH_CHALLANGE) { pMsg = "AUTH_CHALLANGE"; }
		else if(Type == NETMSG_AUTH_RESULT) { pMsg = "AUTH_RESULT"; }
		else if(Type == NETMSG_READY) { pMsg = "READY"; }
		else if(Type == NETMSG_ENTERGAME) { pMsg = "ENTERGAME"; }
		else if(Type == NETMSG_INPUT) { pMsg = "INPUT"; }
		else if(Type == NETMSG_RCON_CMD) { pMsg = "RCON_CMD"; }
		else if(Type == NETMSG_RCON_AUTH) { pMsg = "RCON_AUTH"; }
		else if(Type == NETMSG_REQUEST_MAP_DATA) { pMsg = "REQUEST_MAP_DATA"; }
		else if(Type == NETMSG_AUTH_START) { pMsg = "AUTH_START"; }
		else if(Type == NETMSG_AUTH_RESPONSE) { pMsg = "AUTH_RESPONSE"; }
		else if(Type == NETMSG_PING) { pMsg = "PING"; }
		else if(Type == NETMSG_PING_REPLY) { pMsg = "PING_REPLY"; }
		else if(Type == NETMSG_ERROR) { pMsg = "ERROR"; }
		else if(Type == NETMSG_MAPLIST_ENTRY_ADD) { pMsg = "MAPLIST_ENTRY_ADD"; }
		else if(Type == NETMSG_MAPLIST_ENTRY_REM) { pMsg = "MAPLIST_ENTRY_REM"; }
		dbg_msg("network_out", "pack sys=%d msg=%d (%s)", System, Type, pMsg);
		Reset();
		AddInt((Type<<1)|(System?1:0));
	}
};

#endif
