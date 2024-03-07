/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>

#include "network.h"


int CNetServer::Recv(CNetChunk *pChunk, TOKEN *pResponseToken)
{
	while(1)
	{
		// check for a chunk
		if(m_RecvUnpacker.IsActive() && m_RecvUnpacker.FetchChunk(pChunk))
			return 1;

		NETADDR Addr;
		int Error = UnpackPacket(&Addr, m_RecvUnpacker.m_aBuffer, &m_RecvUnpacker.m_Data);
		// no more packets for now
		if(Error > 0)
			break;
		if(Error)
			continue;

		bool Found = false;
		// try to find matching slot
		for(int i = 0; i < NET_MAX_CLIENTS; i++)
		{
			if(m_aSlots[i].m_Connection.State() == NET_CONNSTATE_OFFLINE)
				continue;

			if(net_addr_comp(m_aSlots[i].m_Connection.PeerAddress(), &Addr, true) == 0)
			{
				if(m_aSlots[i].m_Connection.Feed(&m_RecvUnpacker.m_Data, &Addr))
				{
					if(m_RecvUnpacker.m_Data.m_DataSize)
					{
						if(!(m_RecvUnpacker.m_Data.m_Flags&NET_PACKETFLAG_CONNLESS))
							m_RecvUnpacker.Start(&Addr, &m_aSlots[i].m_Connection, i);
						else
						{
							pChunk->m_Flags = NETSENDFLAG_CONNLESS;
							pChunk->m_Address = *m_aSlots[i].m_Connection.PeerAddress();
							pChunk->m_ClientID = i;
							pChunk->m_DataSize = m_RecvUnpacker.m_Data.m_DataSize;
							pChunk->m_pData = m_RecvUnpacker.m_Data.m_aChunkData;
							if(pResponseToken)
								*pResponseToken = NET_TOKEN_NONE;
							return 1;
						}
					}
				}
				Found = true;
			}
		}

		if(Found)
			continue;

		if(m_RecvUnpacker.m_Data.m_Flags&NET_PACKETFLAG_CONTROL)
		{
			if(m_RecvUnpacker.m_Data.m_aChunkData[0] == NET_CTRLMSG_CONNECT)
			{
				for(int i = 0; i < NET_MAX_CLIENTS; i++)
				{
					if(m_aSlots[i].m_Connection.State() == NET_CONNSTATE_OFFLINE)
					{
						m_NumClients++;
						m_aSlots[i].m_Connection.SetToken(m_RecvUnpacker.m_Data.m_Token);
						m_aSlots[i].m_Connection.Feed(&m_RecvUnpacker.m_Data, &Addr);
						if(m_pfnNewClient)
							m_pfnNewClient(i, m_UserPtr);
						break;
					}
				}
			}
			else if(m_RecvUnpacker.m_Data.m_aChunkData[0] == NET_CTRLMSG_TOKEN)
			{
				dbg_msg("server", "got client token = %d", m_RecvUnpacker.m_Data.m_ResponseToken);
			}
		}
		else if(m_RecvUnpacker.m_Data.m_Flags&NET_PACKETFLAG_CONNLESS)
		{
			pChunk->m_Flags = NETSENDFLAG_CONNLESS;
			pChunk->m_ClientID = -1;
			pChunk->m_Address = Addr;
			pChunk->m_DataSize = m_RecvUnpacker.m_Data.m_DataSize;
			pChunk->m_pData = m_RecvUnpacker.m_Data.m_aChunkData;
			if(pResponseToken)
				*pResponseToken = m_RecvUnpacker.m_Data.m_ResponseToken;
			return 1;
		}
	}
	return 0;
}

int CNetServer::Send(CNetChunk *pChunk, TOKEN Token)
{
	if(pChunk->m_Flags&NETSENDFLAG_CONNLESS)
	{
		if(pChunk->m_DataSize >= NET_MAX_PAYLOAD)
		{
			dbg_msg("netserver", "packet payload too big. %d. dropping packet", pChunk->m_DataSize);
			return -1;
		}

		if(pChunk->m_ClientID == -1)
		{
			for(int i = 0; i < NET_MAX_CLIENTS; i++)
			{
				if(m_aSlots[i].m_Connection.State() == NET_CONNSTATE_OFFLINE)
					continue;

				if(net_addr_comp(&pChunk->m_Address, m_aSlots[i].m_Connection.PeerAddress(), true) == 0)
				{
					// upgrade the packet, now that we know its recipent
					pChunk->m_ClientID = i;
					break;
				}
			}
		}

		if(Token != NET_TOKEN_NONE)
		{
			SendPacketConnless(&pChunk->m_Address, Token, NET_TOKEN_SOME, pChunk->m_pData, pChunk->m_DataSize);
		}
		else
		{
			if(pChunk->m_ClientID == -1)
			{
				SendPacketConnless(&pChunk->m_Address, Token, NET_TOKEN_SOME, pChunk->m_pData, pChunk->m_DataSize);
			}
			else
			{
				dbg_assert(pChunk->m_ClientID >= 0, "errornous client id");
				dbg_assert(pChunk->m_ClientID < NET_MAX_CLIENTS, "errornous client id");
				dbg_assert(m_aSlots[pChunk->m_ClientID].m_Connection.State() != NET_CONNSTATE_OFFLINE, "errornous client id");

				m_aSlots[pChunk->m_ClientID].m_Connection.SendPacketConnless((const char *)pChunk->m_pData, pChunk->m_DataSize);
			}
		}
	}
	else
	{
		if(pChunk->m_DataSize+NET_MAX_CHUNKHEADERSIZE >= NET_MAX_PAYLOAD)
		{
			dbg_msg("netclient", "chunk payload too big. %d. dropping chunk", pChunk->m_DataSize);
			return -1;
		}

		dbg_assert(pChunk->m_ClientID >= 0, "errornous client id");
		dbg_assert(pChunk->m_ClientID < NET_MAX_CLIENTS, "errornous client id");
		dbg_assert(m_aSlots[pChunk->m_ClientID].m_Connection.State() != NET_CONNSTATE_OFFLINE, "errornous client id");

		// pChunk->m_DataSize, pChunk->m_pData
	}
	return 0;
}

