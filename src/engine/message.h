/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_MESSAGE_H
#define ENGINE_MESSAGE_H

#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>

#include <base/dissector/dissector.h>

class CMsgPacker : public CPacker
{
public:
	CMsgPacker(int Type, bool System=false)
	{
		// char aMsg[512];
		// netmsg_to_s(Type, aMsg, sizeof(aMsg));
		// dbg_msg("network_out", "pack sys=%d msg=%d (%s)", System, Type, aMsg);
		Reset();
		AddInt((Type<<1)|(System?1:0));
	}
};

#endif
