/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <generated/protocol.h>

#include <game/client/gameclient.h>

#include "broadcast.h"



void CBroadcast::OnMessage(int MsgType, void* pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_BROADCAST)
	{
		// CNetMsg_Sv_Broadcast *pMsg = (CNetMsg_Sv_Broadcast *)pRawMsg;
	}
}
