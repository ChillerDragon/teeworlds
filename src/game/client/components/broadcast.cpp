/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>

#include "broadcast.h"
#include "chat.h"
#include "scoreboard.h"
#include "motd.h"

static const float BROADCAST_FONTSIZE_BIG = 11.0f;
static const float BROADCAST_FONTSIZE_SMALL = 6.5f;

inline bool IsCharANum(char c)
{
	return c >= '0' && c <= '9';
}

inline bool IsCharWhitespace(char c)
{
	return c == '\n' || c == '\t' || c == ' ';
}

inline int WordLengthBack(const char *pText, int MaxChars)
{
	int s = 0;
	while(MaxChars--)
	{
		if(IsCharWhitespace(*pText))
			return s;
		pText--;
		s++;
	}
	return 0;
}

void CBroadcast::RenderServerBroadcast()
{
}

void CBroadcast::RenderClientBroadcast()
{
}

CBroadcast::CBroadcast()
{
	OnReset();
}

void CBroadcast::DoClientBroadcast(const char *pText)
{
}

void CBroadcast::OnReset()
{
	m_BroadcastTime = 0;
	m_ServerBroadcastReceivedTime = 0;
	m_MuteServerBroadcast = false;
	m_NumSegments = -1;
}

void CBroadcast::OnMessage(int MsgType, void* pRawMsg)
{
	// process server broadcast message
	if(MsgType == NETMSGTYPE_SV_BROADCAST)
	{
		OnBroadcastMessage((CNetMsg_Sv_Broadcast *)pRawMsg);
	}
}

void CBroadcast::OnBroadcastMessage(const CNetMsg_Sv_Broadcast *pMsg)
{
}

void CBroadcast::OnRender()
{
}
