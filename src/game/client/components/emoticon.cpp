/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/ui.h>
#include <game/client/render.h>
#include "emoticon.h"

CEmoticon::CEmoticon()
{
}

void CEmoticon::ConKeyEmoticon(IConsole::IResult *pResult, void *pUserData)
{
}

void CEmoticon::ConEmote(IConsole::IResult *pResult, void *pUserData)
{
}

void CEmoticon::OnConsoleInit()
{
}

void CEmoticon::OnReset()
{
}

void CEmoticon::OnRelease()
{
}

void CEmoticon::OnMessage(int MsgType, void *pRawMsg)
{
}

bool CEmoticon::OnCursorMove(float x, float y, int CursorType)
{
	return true;
}

void CEmoticon::DrawCircle(float x, float y, float r, int Segments)
{
}


void CEmoticon::OnRender()
{
}

void CEmoticon::Emote(int Emoticon)
{
	CNetMsg_Cl_Emoticon Msg;
	Msg.m_Emoticon = Emoticon;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}
