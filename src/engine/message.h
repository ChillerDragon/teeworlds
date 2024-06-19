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
	CMsgPacker(int Type, bool System = false)
	{
		// char aMsg[512];
		// netmsg_to_s(Type, aMsg, sizeof(aMsg));
		// dbg_msg("network_out", "pack sys=%d msg=%d (%s)", System, Type, aMsg);
		Reset();
		if(Type < 0 || Type > 0x3FFFFFFF)
		{
			m_Error = true;
			return;
		}
		AddInt((Type << 1) | (System ? 1 : 0));
	}
};

class CMsgUnpacker : public CUnpacker
{
	int m_Type;
	bool m_System;

public:
	CMsgUnpacker(const void *pData, int Size)
	{
		Reset(pData, Size);
		const int Msg = GetInt();
		if(Msg < 0)
		{
			m_Error = true;
			m_pError = "MsgId is negative";
		}
		if(m_Error)
		{
			m_System = false;
			m_Type = 0;
			return;
		}
		m_System = Msg & 1;
		m_Type = Msg >> 1;
	}

	int Type() const { return m_Type; }
	bool System() const { return m_System; }
};

#endif
