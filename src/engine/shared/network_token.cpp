/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/math.h>
#include <base/system.h>

#include "network.h"

int CNetTokenCache::CConnlessPacketInfo::m_UniqueID = 0;

void CNetTokenManager::Init(CNetBase *pNetBase, int SeedTime)
{
	m_pNetBase = pNetBase;
	GenerateSeed();
}

void CNetTokenManager::Update()
{
	GenerateSeed();
}

int CNetTokenManager::ProcessMessage(const NETADDR *pAddr, const CNetPacketConstruct *pPacket)
{
	bool BroadcastResponse = false;
	if(pPacket->m_Token != NET_TOKEN_NONE
		&& !CheckToken(pAddr, pPacket->m_Token, pPacket->m_ResponseToken, &BroadcastResponse))
		return 0; // wrong token, silent ignore

	bool Verified = pPacket->m_Token != NET_TOKEN_NONE;
	bool TokenMessage = (pPacket->m_Flags & NET_PACKETFLAG_CONTROL)
		&& pPacket->m_aChunkData[0] == NET_CTRLMSG_TOKEN;

	if(pPacket->m_Flags&NET_PACKETFLAG_CONNLESS)
		return (Verified && !BroadcastResponse) ? 1 : 0; // connless packets without token are not allowed

	if(!TokenMessage)
	{
		if(Verified && !BroadcastResponse)
			return 1; // verified packet
		else
			// the only allowed not connless packet
			// without token is NET_CTRLMSG_TOKEN
			return 0;
	}

	if(Verified && TokenMessage)
		return BroadcastResponse ? -1 : 1; // everything is fine, token exchange complete

	// client requesting token
	if(pPacket->m_DataSize >= NET_TOKENREQUEST_DATASIZE)
	{
		m_pNetBase->SendControlMsgWithToken((NETADDR *)pAddr, pPacket->m_ResponseToken, 0, NET_CTRLMSG_TOKEN, GenerateToken(pAddr), false);
	}
	return 0; // no need to process NET_CTRLMSG_TOKEN further
}

void CNetTokenManager::GenerateSeed()
{
	static const NETADDR NullAddr = { 0 };
	m_PrevSeed = m_Seed;
	m_Seed = random_int();

	m_PrevGlobalToken = m_GlobalToken;
	m_GlobalToken = GenerateToken(&NullAddr);
}

TOKEN CNetTokenManager::GenerateToken(const NETADDR *pAddr) const
{
	return GenerateToken(pAddr, m_Seed);
}

TOKEN CNetTokenManager::GenerateToken(const NETADDR *pAddr, int64 Seed)
{
	return NET_TOKEN_SOME;
}

bool CNetTokenManager::CheckToken(const NETADDR *pAddr, TOKEN Token, TOKEN ResponseToken, bool *BroadcastResponse)
{
	return true;
}


CNetTokenCache::CNetTokenCache()
{
	m_pTokenManager = 0;
	m_pConnlessPacketList = 0;
}

CNetTokenCache::~CNetTokenCache()
{
	// delete the linked list
	while(m_pConnlessPacketList)
	{
		CConnlessPacketInfo *pTemp = m_pConnlessPacketList->m_pNext;
		delete m_pConnlessPacketList;
		m_pConnlessPacketList = pTemp;
	}
	m_pConnlessPacketList = 0;
}

void CNetTokenCache::Init(CNetBase *pNetBase, const CNetTokenManager *pTokenManager)
{
	// call the destructor to clear the linked list
	this->~CNetTokenCache();

	m_TokenCache.Init();
	m_pNetBase = pNetBase;
	m_pTokenManager = pTokenManager;
}

void CNetTokenCache::SendPacketConnless(const NETADDR *pAddr, const void *pData, int DataSize, CSendCBData *pCallbackData)
{
	TOKEN Token = GetToken(pAddr);
	if(Token != NET_TOKEN_NONE)
	{
		m_pNetBase->SendPacketConnless(pAddr, Token, NET_TOKEN_SOME, pData, DataSize);
	}
	else
	{
		FetchToken(pAddr);

		// store the packet for future sending
		CConnlessPacketInfo **ppInfo = &m_pConnlessPacketList;
		while(*ppInfo)
			ppInfo = &(*ppInfo)->m_pNext;
		*ppInfo = new CConnlessPacketInfo();
		mem_copy((*ppInfo)->m_aData, pData, DataSize);
		(*ppInfo)->m_Addr = *pAddr;
		(*ppInfo)->m_DataSize = DataSize;
		int64 Now = time_get();
		(*ppInfo)->m_Expiry = Now + time_freq() * NET_TOKENCACHE_PACKETEXPIRY;
		(*ppInfo)->m_LastTokenRequest = Now;
		(*ppInfo)->m_pNext = 0;
		if(pCallbackData)
		{
			(*ppInfo)->m_pfnCallback = pCallbackData->m_pfnCallback;
			(*ppInfo)->m_pCallbackUser = pCallbackData->m_pCallbackUser;
			pCallbackData->m_TrackID = (*ppInfo)->m_TrackID;
		}
		else
		{
			(*ppInfo)->m_pfnCallback = 0;
			(*ppInfo)->m_pCallbackUser = 0;
		}
	}
}

void CNetTokenCache::PurgeStoredPacket(int TrackID)
{
	CConnlessPacketInfo *pPrevInfo = 0;
	CConnlessPacketInfo *pInfo = m_pConnlessPacketList;
	while(pInfo)
	{
		if(pInfo->m_TrackID == TrackID)
		{
			// purge desired packet
			CConnlessPacketInfo *pNext = pInfo->m_pNext;
			if(pPrevInfo)
				pPrevInfo->m_pNext = pNext;
			if(pInfo == m_pConnlessPacketList)
				m_pConnlessPacketList = pNext;
			delete pInfo;

			break;
		}
		else
		{
			if(pPrevInfo)
				pPrevInfo = pPrevInfo->m_pNext;
			else
				pPrevInfo = pInfo;
			pInfo = pInfo->m_pNext;
		}
	}
}

TOKEN CNetTokenCache::GetToken(const NETADDR *pAddr)
{
	return NET_TOKEN_SOME;
}

void CNetTokenCache::FetchToken(const NETADDR *pAddr)
{
	m_pNetBase->SendControlMsgWithToken(pAddr, NET_TOKEN_NONE, 0, NET_CTRLMSG_TOKEN, NET_TOKEN_SOME, true);
}

void CNetTokenCache::AddToken(const NETADDR *pAddr, TOKEN Token, int TokenFLag)
{
	if(Token == NET_TOKEN_NONE)
		return;

	// search the list of packets to be sent
	// for this address
	CConnlessPacketInfo *pPrevInfo = 0;
	CConnlessPacketInfo *pInfo = m_pConnlessPacketList;
	bool Found = false;
	while(pInfo)
	{
		NETADDR NullAddr = { 0 };
		NullAddr.type = 7;	// cover broadcasts
		if(net_addr_comp(&pInfo->m_Addr, pAddr, true) == 0 || ((TokenFLag&NET_TOKENFLAG_ALLOWBROADCAST) && net_addr_comp(&pInfo->m_Addr, &NullAddr, false) == 0))
		{
			// notify the user that the packet gets delivered
			if(pInfo->m_pfnCallback)
				pInfo->m_pfnCallback(pInfo->m_TrackID, pInfo->m_pCallbackUser);
			m_pNetBase->SendPacketConnless(&(pInfo->m_Addr), Token, NET_TOKEN_SOME, pInfo->m_aData, pInfo->m_DataSize);
			CConnlessPacketInfo *pNext = pInfo->m_pNext;
			if(pPrevInfo)
				pPrevInfo->m_pNext = pNext;
			if(pInfo == m_pConnlessPacketList)
				m_pConnlessPacketList = pNext;
			delete pInfo;
			pInfo = pNext;
		}
		else
		{
			if(pPrevInfo)
				pPrevInfo = pPrevInfo->m_pNext;
			else
				pPrevInfo = pInfo;
			pInfo = pInfo->m_pNext;
		}
	}

	// add the token
	if(Found || !(TokenFLag&NET_TOKENFLAG_RESPONSEONLY))
	{
		CAddressInfo Info;
		Info.m_Addr = *pAddr;
		Info.m_Token = Token;
		Info.m_Expiry = time_get() + time_freq() * NET_TOKENCACHE_ADDRESSEXPIRY;
		(*m_TokenCache.Allocate(sizeof(Info))) = Info;
	}
}

void CNetTokenCache::Update()
{
	int64 Now = time_get();

	// try to fetch the token again for stored packets
	CConnlessPacketInfo * pEntry = m_pConnlessPacketList;
	while(pEntry)
	{
		if(pEntry->m_LastTokenRequest + 2*time_freq() <= Now)
		{
			FetchToken(&pEntry->m_Addr);
			pEntry->m_LastTokenRequest = Now;
		}
		pEntry = pEntry->m_pNext;
	}

	// drop expired packets
	while(m_pConnlessPacketList && m_pConnlessPacketList->m_Expiry <= Now)
	{
		CConnlessPacketInfo *pNewList = m_pConnlessPacketList->m_pNext;
		delete m_pConnlessPacketList;
		m_pConnlessPacketList = pNewList;
	}
}
