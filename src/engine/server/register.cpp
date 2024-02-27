/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <engine/shared/network.h>
#include <engine/shared/config.h>

#include "register.h"

CRegister::CRegister()
{
	m_pNetServer = 0;
	m_pConfig = 0;

	m_RegisterState = REGISTERSTATE_START;
	m_RegisterStateStart = 0;
	m_RegisterFirst = 1;
	m_RegisterCount = 0;

	m_RegisterRegisteredServer = -1;
}

void CRegister::RegisterNewState(int State)
{
	m_RegisterState = State;
	m_RegisterStateStart = time_get();
}

void CRegister::RegisterSendHeartbeat(NETADDR Addr)
{
}

void CRegister::RegisterSendCountRequest(NETADDR Addr)
{
}

void CRegister::RegisterGotCount(CNetChunk *pChunk)
{
}

void CRegister::Init(CNetServer *pNetServer, CConfig *pConfig)
{
	m_pNetServer = pNetServer;
	m_pConfig = pConfig;
}

void CRegister::RegisterUpdate(int Nettype)
{
}

int CRegister::RegisterProcessPacket(CNetChunk *pPacket, TOKEN Token)
{
	return 0;
}
